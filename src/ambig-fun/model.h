#pragma once

#include <util/common/geom/point.h>
#include <util/common/math/vec.h>
#include <util/common/plot/plot.h>
#include <util/common/math/fuzzy.h>
#include <util/common/math/complex.h>

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <atomic>

#include <omp.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif // !M_PI

namespace model
{

    /*****************************************************/
    /*                     params                        */
    /*****************************************************/

	struct parameters
	{
        double carrier;
        double sampling_rate;
		double bitrate;
		size_t N;
		double tau; // 0..1
		double doppler;
		double snr;
		double tau_from, tau_to; size_t tau_count;
        double dopp_from, dopp_to; size_t dopp_count;
		int num_of_tests;
	};

	inline parameters make_default_parameters()
	{
		parameters p =
		{
			0,
			9600 * 3,
			9600,
            64,
            0.5,
            100.0,
			-10,
			0.0, 0.5, 20,
			0.0, 200.0, 20,
			10
		};
		return p;
	}

    /*****************************************************/
    /*                     data                          */
    /*****************************************************/

    class cancellation_token
    {
    private:
        mutable std::shared_ptr < std::atomic_bool > _cancelled;
    public:
        cancellation_token()
            : _cancelled(std::make_shared < std::atomic_bool > ())
        {
        }
    public:
        void cancel() const
        {
            *_cancelled = true;
        }
        bool is_cancelled() const
        {
            return *_cancelled;
        }
    public:
        operator bool() const { return is_cancelled(); }
    };

    using ct_t = cancellation_token;

    enum modulation_t
    {
        AM, PM, FM
    };

    using signal_t = std::vector < geom::point < double, math::complex<> > > ;

    template < typename T >
    struct sigtuple_t
    {
        T am, pm, fm;
    };

    using signals_t = sigtuple_t < signal_t > ;

    using stats_t = sigtuple_t < double > ;

    struct signals_pair
    {
        signals_t base;
        signals_t recv;
    };

    struct recv_params
    {
        double tau;
        double doppler;
    };

    inline recv_params from_params(const parameters & p)
    {
        return {
            p.tau,
            p.doppler
        };
    }

    template < typename T >
    struct slice
    {
        std::vector < T > & data;
        size_t offset;
        size_t len;
        slice(std::vector < T > & d, size_t o, size_t l)
            : data(d), offset(o), len(l)
        {
        }
        T operator[] (size_t i) { return data[i + offset]; }
    };

    template < typename T >
    inline slice < T > make_slice(std::vector < T > & d, size_t o, size_t l)
    {
        return slice < T > (d, o, l);
    }

    inline size_t boole(bool b) { return b ? 1 : 0; }

    inline void modulate
    (
        slice < bool > bits,
        signals_t & r,
        const parameters & p,
        double dopp
    )
    {
        using math::_i;

        ASSERT((bits.len & 1) == 0);

        double ts = (double)bits.len / p.bitrate;
        size_t len = std::floor(ts * p.sampling_rate);

        r.am.resize(len);
        r.pm.resize(len);
        r.fm.resize(len);

        #pragma omp parallel for
        for (int i = 0; i < (int)len; ++i)
        {
            double t = i / p.sampling_rate;
            size_t bit_idx = std::round(t * p.bitrate);
            size_t bit = boole(bits[bit_idx]);

            math::complex<> val;
            math::complex<> doppler = std::exp(- 2 * M_PI * _i * dopp * t);
            math::complex<> base = std::exp(- 2 * M_PI * _i * p.carrier * t);

            {
                val = bit;
                r.am[i] = { t, base * val * doppler };
            }
            {
                val = std::exp(M_PI * _i * bit);
                r.pm[i] = { t, base * val * doppler };
            }
            {
                double df = (bit ? 1 : -1) * p.bitrate / 2;
                double dphi = bit;
                val = std::exp(- (2 * M_PI * _i * df * t + M_PI * _i * dphi));
                r.fm[i] = { t, base * val * doppler };
            }
        }
    }

    inline math::complex<> noise()
    {
        using math::_i;

        auto _1 = 1 + _i;

        math::complex<> r;
        for (size_t i = 0; i < 12; ++i)
        {
            r = r + (rand() + _i * rand()) / RAND_MAX - 0.5 * _1;
        }
        return r;
    }

    inline void noisify(signals_t & r, double snr)
    {
        double es2en = std::exp(snr / 10.0);

        double e_n = 0, e_am = 0, e_pm = 0, e_fm = 0;
        std::vector < math::complex<> > n(r.am.size());
            
        #pragma omp parallel for reduction(+:e_n,e_am,e_pm,e_fm)
        for (int i = 0; i < (int)r.am.size(); ++i)
        {
            auto n0 = noise(); n[i] = n0;
            e_n += math::sqnorm(n0);
            e_am += math::sqnorm(r.am[i].y);
            e_pm += math::sqnorm(r.pm[i].y);
            e_fm += math::sqnorm(r.fm[i].y);
        }

        #pragma omp parallel for
        for (int i = 0; i < (int)r.am.size(); ++i)
        {
            r.am[i].y = r.am[i].y + (e_am / es2en / e_n) * n[i];
            r.pm[i].y = r.pm[i].y + (e_pm / es2en / e_n) * n[i];
            r.fm[i].y = r.fm[i].y + (e_fm / es2en / e_n) * n[i];
        }
    }

    inline void gen_signals
    (
        signals_pair & r,
        const parameters & p,
        const recv_params & rp
    )
    {
        ASSERT((p.N & 1) == 0);

        double base_ts = (double)p.N / p.bitrate;

        ASSERT(0 <= rp.tau && rp.tau < 1);

        size_t recv_off = std::floor(rp.tau * base_ts * p.bitrate);

        std::vector < bool > bits(2 * p.N + 1);
        for (size_t i = 0; i < bits.size(); ++i)
            bits[i] = (rand() < RAND_MAX / 2);

        auto base_bits = make_slice(bits, recv_off, p.N);
        auto recv_bits = make_slice(bits, 0, 2 * p.N);

        modulate(base_bits, r.base, p, 0);
        noisify(r.base, +10.0);
        modulate(recv_bits, r.recv, p, rp.doppler);
        noisify(r.recv, p.snr);
    }

    inline stats_t correlate
    (
        const signals_pair & s,
        signals_t & r,
        double dopp,
        bool normalize,
        ct_t ct
    )
    {
        r.am.clear(); r.pm.clear(); r.fm.clear();
        r.am.resize(s.base.am.size());
        r.pm.resize(s.base.am.size());
        r.fm.resize(s.base.am.size());

        double am_max = -1, pm_max = -1, fm_max = -1;
        double am_std = 0, pm_std = 0, fm_std = 0;
        double am_mean = 0, pm_mean = 0, fm_mean = 0;

        std::vector < math::complex<> > doppler(s.recv.am.size());
        #pragma omp parallel for
        for (int i = 0; i < (int)s.recv.am.size(); ++i)
        {
            doppler[i] = std::exp(math::_i * 2 * M_PI * dopp * s.recv.am[i].x);
        }

        #pragma omp parallel for reduction(+:am_std,pm_std,fm_std,am_mean,pm_mean,fm_mean)
        for (int i = 0; i < (int)s.base.am.size(); ++i)
        {
            // cannot break due to parallel for,
            // just omit real calculations instead
            if (ct) continue;

            for (size_t j = 0; j < s.base.am.size(); ++j)
            {
                r.am[i] = { s.base.am[i].x, r.am[i].y + s.base.am[j].y * math::conjugate(s.recv.am[i + j].y * doppler[i + j]) };
                r.pm[i] = { s.base.am[i].x, r.pm[i].y + s.base.pm[j].y * math::conjugate(s.recv.pm[i + j].y * doppler[i + j]) };
                r.fm[i] = { s.base.am[i].x, r.fm[i].y + s.base.fm[j].y * math::conjugate(s.recv.fm[i + j].y * doppler[i + j]) };
            }
            double am_cur = (r.am[i].y = math::norm(r.am[i].y)).re;
            double pm_cur = (r.pm[i].y = math::norm(r.pm[i].y)).re;
            double fm_cur = (r.fm[i].y = math::norm(r.fm[i].y)).re;
            am_std += am_cur * am_cur;
            pm_std += pm_cur * pm_cur;
            fm_std += fm_cur * fm_cur;
            am_mean += am_cur;
            pm_mean += pm_cur;
            fm_mean += fm_cur;
            #pragma omp critical
            {
                if (am_max < am_cur) am_max = am_cur;
                if (pm_max < pm_cur) pm_max = pm_cur;
                if (fm_max < fm_cur) fm_max = fm_cur;
            }
        }

        am_mean /= s.base.am.size();
        pm_mean /= s.base.am.size();
        fm_mean /= s.base.am.size();
        am_std = std::sqrt(am_std / s.base.am.size() - am_mean * am_mean);
        pm_std = std::sqrt(pm_std / s.base.am.size() - pm_mean * pm_mean);
        fm_std = std::sqrt(fm_std / s.base.am.size() - fm_mean * fm_mean);

        if (normalize)
        {
            #pragma omp parallel for
            for (int i = 0; i < (int)s.base.am.size(); ++i)
            {
                r.am[i].y = r.am[i].y / am_max;
                r.pm[i].y = r.pm[i].y / pm_max;
                r.fm[i].y = r.fm[i].y / fm_max;
            }
        }

        return {
            (am_max - am_mean) / am_std,
            (pm_max - pm_mean) / pm_std,
            (fm_max - fm_mean) / fm_std
        };
    }

    inline void quality
    (
        signals_t & r,
        const parameters & p,
        ct_t ct,
        const std::function < void() > & cb
    )
    {
        signals_t c;

        double ddopp = (p.dopp_to - p.dopp_from) / p.dopp_count;

        r.am.clear(); r.pm.clear(); r.fm.clear();
        r.am.resize(p.dopp_count);
        r.pm.resize(p.dopp_count);
        r.fm.resize(p.dopp_count);

        for (size_t k = 0; k < p.num_of_tests; ++k)
        {
            if (ct) break;
            for (size_t i = 0; i < p.dopp_count; ++i)
            {
                if (ct) break;

                double dopp = p.dopp_from + ddopp * i;

                signals_pair s;
                recv_params rp = from_params(p);
                rp.doppler = dopp;
                gen_signals(s, p, rp);

                if (ct) break;

                auto stats = correlate(s, c, 0, false, ct);
                r.am[i] = { dopp, r.am[i].y * k / (k + 1) + stats.am / (k + 1) };
                r.pm[i] = { dopp, r.pm[i].y * k / (k + 1) + stats.pm / (k + 1) };
                r.fm[i] = { dopp, r.fm[i].y * k / (k + 1) + stats.fm / (k + 1) };
            }
            if (ct) break;
            cb();
        }
    }

    /*****************************************************/
    /*                     drawing                       */
    /*****************************************************/
	
    using points_t = std::vector < geom::point2d_t > ;

    struct plot_data
    {
        util::ptr_t < points_t > data;
        plot::list_drawable < points_t > :: ptr_t plot;
    };

    struct complex_plot_data
    {
        plot_data re, im;
    };

	struct complex_plot_group_data
	{
		plot::auto_viewport < points_t > ::ptr_t autoworld;
		complex_plot_data am, pm, fm;
	};

	struct single_plot_group_data
	{
		plot::auto_viewport < points_t > ::ptr_t autoworld;
		plot_data am, pm, fm;
	};

    struct model_data
    {
        util::ptr_t < parameters > params;

        complex_plot_group_data signals;
		complex_plot_group_data signals_shifted;
		single_plot_group_data correlation;
		single_plot_group_data quality;
    };

    inline static plot_data make_plot_data
    (
        plot::palette::pen_ptr pen = plot::palette::pen(0xffffff),
        plot::list_data_format data_format = plot::list_data_format::chain
    )
    {
        plot_data pd;
        pd.data = util::create < points_t > ();
        pd.plot = plot::list_drawable < points_t > :: create
        (
            plot::make_data_source(pd.data),
            nullptr, // no point painter
            pen
        );
        pd.plot->data_format = data_format;
        return pd;
    }

	inline static complex_plot_group_data make_plot_group_data
	(
		plot::palette::pen_ptr am_re = plot::palette::pen(0x5555ff, 2),
		plot::palette::pen_ptr pm_re = plot::palette::pen(0x55aa55, 2),
		plot::palette::pen_ptr fm_re = plot::palette::pen(0xff0000, 2),
		plot::palette::pen_ptr am_im = plot::palette::pen(0x111166),
		plot::palette::pen_ptr pm_im = plot::palette::pen(0x114411),
		plot::palette::pen_ptr fm_im = plot::palette::pen(0x660000),
		plot::list_data_format data_format = plot::list_data_format::chain
	)
	{
		complex_plot_group_data pd;
		pd.autoworld = plot::min_max_auto_viewport < points_t > ::create();
		pd.am.re = make_plot_data(am_re, data_format);
		pd.pm.re = make_plot_data(pm_re, data_format);
		pd.fm.re = make_plot_data(fm_re, data_format);
		pd.am.im = make_plot_data(am_im, data_format);
		pd.pm.im = make_plot_data(pm_im, data_format);
		pd.fm.im = make_plot_data(fm_im, data_format);
		return pd;
	}

	inline static single_plot_group_data make_single_plot_group_data
	(
		plot::palette::pen_ptr am_re = plot::palette::pen(0x5555ff, 2),
		plot::palette::pen_ptr pm_re = plot::palette::pen(0x55aa55, 2),
		plot::palette::pen_ptr fm_re = plot::palette::pen(0xff0000, 2),
		plot::list_data_format data_format = plot::list_data_format::chain
	)
	{
		single_plot_group_data pd;
		pd.autoworld = plot::min_max_auto_viewport < points_t > ::create();
		pd.am = make_plot_data(am_re, data_format);
		pd.pm = make_plot_data(pm_re, data_format);
		pd.fm = make_plot_data(fm_re, data_format);
		return pd;
	}

    template < typename PlotGroupData >
    inline static plot::drawable::ptr_t make_root_drawable
    (
        const PlotGroupData & p,
        std::vector < plot::drawable::ptr_t > layers
    )
    {
        using namespace plot;

        return viewporter::create(
            tick_drawable::create(
                layer_drawable::create(layers),
                const_n_tick_factory<axe::x>::create(
                    make_simple_tick_formatter(6, 8),
                    0,
                    5
                ),
                const_n_tick_factory<axe::y>::create(
                    make_simple_tick_formatter(6, 8),
                    0,
                    5
                ),
                palette::pen(RGB(80, 80, 80)),
                RGB(200, 200, 200)
            ),
            make_viewport_mapper(make_world_mapper < points_t > (p.autoworld))
        );
    }

    inline model_data make_model_data(const parameters & p = make_default_parameters())
    {
        model_data md;
        md.params = util::create < parameters > (p);
        md.signals = make_plot_group_data();
		md.signals_shifted = make_plot_group_data();
		md.correlation = make_single_plot_group_data();
		md.quality = make_single_plot_group_data();
        return md;
    }

    inline void fill_complex(complex_plot_group_data & pg, complex_plot_data & pd, const signal_t & s)
    {
        pd.re.data->resize(s.size()); pd.im.data->resize(s.size());
        for (size_t i = 0; i < s.size(); ++i)
        {
            pd.re.data->at(i) = { s[i].x, s[i].y.re };
            pd.im.data->at(i) = { s[i].x, s[i].y.im };
        }
        pg.autoworld->adjust(*pd.re.data);
        pg.autoworld->adjust(*pd.im.data);
    }

    inline void fill_re(single_plot_group_data & pg, plot_data & pd, const signal_t & s)
    {
        pd.data->resize(s.size()); pd.data->resize(s.size());
        for (size_t i = 0; i < s.size(); ++i)
        {
            pd.data->at(i) = { s[i].x, s[i].y.re };
        }
        pg.autoworld->adjust(*pd.data);
    }

    inline void fill_signals(model_data & md, const signals_pair & p)
    {
        md.signals.autoworld->clear();
        md.signals_shifted.autoworld->clear();
        fill_complex(md.signals, md.signals.am, p.base.am);
        fill_complex(md.signals, md.signals.pm, p.base.pm);
        fill_complex(md.signals, md.signals.fm, p.base.fm);
        fill_complex(md.signals_shifted, md.signals_shifted.am, p.recv.am);
        fill_complex(md.signals_shifted, md.signals_shifted.pm, p.recv.pm);
        fill_complex(md.signals_shifted, md.signals_shifted.fm, p.recv.fm);
        md.signals.autoworld->flush();
        md.signals_shifted.autoworld->flush();
    }

    inline void fill_corr(model_data & md, const signals_t & p)
    {
        md.correlation.autoworld->clear();
        fill_re(md.correlation, md.correlation.am, p.am);
        fill_re(md.correlation, md.correlation.pm, p.pm);
        fill_re(md.correlation, md.correlation.fm, p.fm);
        md.correlation.autoworld->flush();
    }

    inline void fill_qual(model_data & md, const signals_t & p)
    {
        md.quality.autoworld->clear();
        fill_re(md.quality, md.quality.am, p.am);
        fill_re(md.quality, md.quality.pm, p.pm);
        fill_re(md.quality, md.quality.fm, p.fm);
        md.quality.autoworld->flush();
    }
}
