#pragma once

#include <vector>

#include <util/common/geom/point.h>
#include <util/common/plot/shape.h>
#include <util/common/gui/OglControl.h>

// CSurfacePlotControl

class CSurfacePlotControl : public COglControl
{
    DECLARE_DYNAMIC(CSurfacePlotControl)

public:

    using grid_t = std::vector < std::vector < geom::point < double > > > ;
    using mat_t = std::vector < std::vector < double > > ;

public:

    std::shared_ptr < grid_t > points;
    std::vector < std::shared_ptr < mat_t > > values;
    std::vector < COLORREF > accents;
    size_t visible_layer;
    double color_factor;
    std::function < void () > custom_painter;

public:
    CSurfacePlotControl();
    virtual ~CSurfacePlotControl();
    virtual void OnDrawItemOGL();

protected:
    DECLARE_MESSAGE_MAP()
};
