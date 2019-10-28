// SurfacePlotControl.cpp : implementation file
//

#include "stdafx.h"
#include "SurfacePlotControl.h"

#include <gl/GLU.h>
#include <gl/GL.h>

static void _interpolate_color(double value, COLORREF accent, GLfloat *color)
{
    float r = GetRValue(accent), g = GetGValue(accent), b = GetBValue(accent);
    r /= 255; g /= 255; b /= 255;
    const float aR = r;   const float aG = g; const float aB = b;
    const float bR = 0.8; const float bG = 1; const float bB = 1;
    const float cR = 0;   const float cG = 0; const float cB = 1;

    if (value > 1)  value = 1;
    if (value < -1) value = -1;

    if (value > 0)
    {
        color[0] = (float)((aR - bR) * value) + bR;
        color[1] = (float)((aG - bG) * value) + bG;
        color[2] = (float)((aB - bB) * value) + bB;
    }
    else
    {
        color[0] = (float)((cR - bR) * (-value)) + bR;
        color[1] = (float)((cG - bG) * (-value)) + bG;
        color[2] = (float)((cB - bB) * (-value)) + bB;
    }
}


// CSurfacePlotControl

IMPLEMENT_DYNAMIC(CSurfacePlotControl, COglControl)

CSurfacePlotControl::CSurfacePlotControl()
    : color_factor(1)
    , visible_layer(0)
{
}

CSurfacePlotControl::~CSurfacePlotControl()
{
}


BEGIN_MESSAGE_MAP(CSurfacePlotControl, COglControl)
END_MESSAGE_MAP()



// CSurfacePlotControl message handlers

void CSurfacePlotControl::OnDrawItemOGL()
{
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    m_zoomMultiplier = 0.1;
    m_translateXMultiplier = 1;
    m_translateYMultiplier = 1;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2, 2, -2, 2, -200, 200);
    glTranslated(GetTranslateX(), GetTranslateY(), -1);
    glRotated(GetRotAngleH() / M_PI * 180, 1, 0, 0);
    glRotated(GetRotAngleV() / M_PI * 180, 0, 0, 1);
    glScaled(GetZoom(), GetZoom(), GetZoom());

    glMatrixMode(GL_MODELVIEW);

    glClearColor(0, 0, 0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat color[3 * 4];

    if (visible_layer < values.size())
    {
        auto & points = *this->points;
        auto & values = *this->values[visible_layer];
        auto & accent = this->accents[visible_layer];

        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i + 1 < points.size(); ++i)
        for (size_t j = 0; j + 1 < points[i].size(); ++j)
        {
            _interpolate_color(color_factor * values[i][j], accent, color);
            _interpolate_color(color_factor * values[i + 1][j], accent, color + 3);
            _interpolate_color(color_factor * values[i][j + 1], accent, color + 6);
            _interpolate_color(color_factor * values[i + 1][j + 1], accent, color + 9);

            glColor3fv(color); glVertex3d(points[i][j].x, points[i][j].y, values[i][j]);
            glColor3fv(color + 3); glVertex3d(points[i + 1][j].x, points[i + 1][j].y, values[i + 1][j]);
            glColor3fv(color + 9); glVertex3d(points[i + 1][j + 1].x, points[i + 1][j + 1].y, values[i + 1][j + 1]);
            glColor3fv(color); glVertex3d(points[i][j].x, points[i][j].y, values[i][j]);
            glColor3fv(color + 6); glVertex3d(points[i][j + 1].x, points[i][j + 1].y, values[i][j + 1]);
            glColor3fv(color + 9); glVertex3d(points[i + 1][j + 1].x, points[i + 1][j + 1].y, values[i + 1][j + 1]);
        }
        glEnd();

        if (custom_painter) custom_painter();
    }

    glFinish();
}
