#pragma once

#include <util/common/geom/point.h>
#include <util/common/math/vec.h>
#include <util/common/plot/plot.h>
#include <util/common/math/fuzzy.h>

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <array>

#include <omp.h>

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
			25000,
			250000,
			9600,
            64,
            0.5,
            100.0,
			-10,
			0.0, 0.5, 20,
			0.0, 1000.0, 20,
			10
		};
		return p;
	}

    /*****************************************************/
    /*                     data                          */
    /*****************************************************/

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

	struct plot_group_data
	{
		plot::world_t::ptr_t world;
		plot::auto_viewport < points_t > ::ptr_t autoworld;
		complex_plot_data am, pm, fm;
	};

    struct model_data
    {
        util::ptr_t < parameters > params;

        plot_group_data signals;
		plot_group_data signals_shifted;
		plot_group_data correlation;
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

	inline static plot_group_data make_plot_group_data
	(
		plot::palette::pen_ptr am_re = plot::palette::pen(0x5555ff),
		plot::palette::pen_ptr pm_re = plot::palette::pen(0x55aa55),
		plot::palette::pen_ptr fm_re = plot::palette::pen(0xff0000),
		plot::palette::pen_ptr am_im = plot::palette::pen(0x3333aa),
		plot::palette::pen_ptr pm_im = plot::palette::pen(0x338833),
		plot::palette::pen_ptr fm_im = plot::palette::pen(0xaa0000),
		plot::list_data_format data_format = plot::list_data_format::chain
	)
	{
		plot_group_data pd;
		pd.world = plot::world_t::create();
		pd.autoworld = plot::min_max_auto_viewport < points_t > ::create();
		pd.am.re = make_plot_data(am_re, data_format);
		pd.pm.re = make_plot_data(pm_re, data_format);
		pd.fm.re = make_plot_data(fm_re, data_format);
		pd.am.im = make_plot_data(am_im, data_format);
		pd.pm.im = make_plot_data(pm_im, data_format);
		pd.fm.im = make_plot_data(fm_im, data_format);
		return pd;
	}

    inline static plot::drawable::ptr_t make_root_drawable
    (
        const plot_group_data & p,
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
		md.correlation = make_plot_group_data();
        return md;
    }
}
