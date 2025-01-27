/*
 * @Author: SPeak Shen 
 * @Date: 2021-11-13 23:13:11 
 * @Last Modified by: SPeak Shen
 * @Last Modified time: 2021-11-20 23:43:45
 * 
 * Tiny UEFI Graphical User Interface
 * 
 */

#ifndef __TUGUI_HPP__
#define __TUGUI_HPP__

// TMATH
#include <formula.hpp>
#include <vector.hpp>
#include <homocoordinates.hpp>

#include <utils.hpp>

#include <platform/base_interface.hpp>
#include <platform/event_interface.hpp>
//#include <core/colors/color.hpp>
//#include <core/transform/AffineTransform.hpp>

namespace TUGUI {

using Math::Formula::LinearEquation;
using MUTILS::max;
using MUTILS::min;
using MUTILS::swap;

class Base {
public:

    Base() = default;

    void clearScreen() {
        INTERFACE::gBaseInterfacePtr->clearScrean();
    }

    void drawPixel(unsigned int x, unsigned int y, RGB rgb) {
        INTERFACE::gBaseInterfacePtr->drawPixel(x, y, rgb);
    }

    void drawPixel(const TMATH::HomoCoordinates<3> &point, RGB rgb) {
        INTERFACE::gBaseInterfacePtr->drawPixel(point[0], point[1], rgb);
    }

    void drawXLine(unsigned int y, unsigned int x1, unsigned int x2, RGB rgb = PIXEL_WHITE) {
        if (x1 > x2) swap(x1, x2);
        unsigned int end = min(x2, INTERFACE::gBaseInterfacePtr->getHorizontalResolution());
        while (x1 <= end) {
            drawPixel(x1, y, rgb);
            x1++;
        }
    }

    void drawYLine(const unsigned int &x, unsigned int y1, unsigned int y2, RGB rgb = PIXEL_WHITE) {
        if (y1 > y2) swap(y1, y2);
        unsigned int end = min(y2, INTERFACE::gBaseInterfacePtr->getVerticalResolution());
        while (y1 <= end) {
            drawPixel(x, y1, rgb);
            y1++;
        }
    }

    void drawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, RGB rgb = PIXEL_WHITE) {
        if (x1 == x2) drawYLine(x1, y1, y2, rgb);
        if (y1 == y2) drawXLine(y1, x1, x2, rgb);

        __mLinearEquation.set(x1, y1, x2, y2);
        
        double sX { x1 * 1. }, dX { x2 * 1. };
        double deltaX { 1 };

        if (sX > dX) swap(sX, dX);
        
        if (__mLinearEquation.getSlope() > 1 || __mLinearEquation.getSlope() < -1) // deltaY = 1 or -1
            deltaX = 1. / MUTILS::abs(__mLinearEquation.getSlope());

        while (sX < dX) {
            drawPixel(sX, __mLinearEquation.getY(sX), rgb);
            sX += deltaX;
        }
    }

    void drawLine(const TMATH::HomoCoordinates<3> &begin, const TMATH::HomoCoordinates<3> &end, RGB rgb = PIXEL_WHITE) {
        if (begin[0] == end[0]) drawYLine(begin[0], begin[1], end[1], rgb);
        if (begin[1] == end[1]) drawXLine(begin[1], begin[0], end[0], rgb);

        __mLinearEquation.set(begin[0], begin[1], end[0], end[1]);
        
        double sX { begin[0] * 1. }, dX { end[0] * 1. };
        double deltaX { 1 };

        if (sX > dX) swap(sX, dX);
        
        if (__mLinearEquation.getSlope() > 1 || __mLinearEquation.getSlope() < -1) // deltaY = 1 or -1
            deltaX = 1. / MUTILS::abs(__mLinearEquation.getSlope());

        while (sX < dX) {
            drawPixel(sX, __mLinearEquation.getY(sX), rgb);
            sX += deltaX;
        }
    }

    void puts(wchar_t* s) {
        INTERFACE::gBaseInterfacePtr->tuguiOutputString(s);
    }

    void putc(wchar_t c) {
        wchar_t str[2] = {L" "};
        str[0] = c;
        INTERFACE::gBaseInterfacePtr->tuguiOutputString(str);
    }

    wchar_t getc(void) {
        EFI_INPUT_KEY key;
        unsigned long long waitidx;

        INTERFACE::gEventInterfacePtr->waitForKeyEvent(1, &waitidx);
        while (INTERFACE::gEventInterfacePtr->readKeyStrokeEvent(&key))
            ;

        return static_cast<wchar_t>(key.UnicodeChar);
    }

    unsigned int gets(wchar_t *buf, unsigned int buf_size) {
        unsigned int i;

        for (i = 0; i < buf_size - 1;) {
            buf[i] = getc();
            putc(buf[i]);
            if (buf[i] == L'\r') {
                putc(L'\n');
                break;
            }
            i++;
        }
        buf[i] = L'\0';

        return i;
    }

   private:
    static LinearEquation __mLinearEquation;

};  /* GUI end */


class Graphics : public Base{
public:
    struct Rectangle {
        unsigned int x;
        unsigned int y;
        unsigned int w;
        unsigned int h;
    };

public:

    void drawRectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, RGB rgb = PIXEL_WHITE);

    void drawRectangle(Rectangle rect, RGB rgb = PIXEL_WHITE);

    void drawTriangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, RGB rgb);

    void drawTriangle(const TMATH::HomoCoordinates<3> &,
       const TMATH::HomoCoordinates<3> &, const TMATH::HomoCoordinates<3> &, RGB rgb = PIXEL_WHITE);

    void fillTriangle( int x1,  int y1,  int x2,  int y2,  int x3,  int y3, RGB rgb = PIXEL_WHITE);
    /**
     *  F(x, y) = x^2 + y^2 -R^2 
     *  1/8 circle of first quadrant(above)
     *  deltaX = 1;
     *  deltaY = 0.5;
     *  compare F(x + deltaX, y - deltaY) with 0 to choice (x + 1, y) and (x, y - 1)
     *  F(x + 1, y - 0.5) = x^2 + 2x + 1 + y2 - y + 0.25 - R^2
     *  Example:
     *      when initial point is (0, R),
     *      F(x + 1, y - 0.5) = 1.25 - R
     *      if (1.25 - R <= 0) choice (x + 1, y)
     *      else choice (x, y - 1)
     *          *---->(x + 1, y)
     *          |
     *          V
     *      (x, y - 1)
    */
    void drawCircle(unsigned int x0, unsigned int y0, unsigned int r, RGB rgb = PIXEL_WHITE);

    void tuguiblt(unsigned char img[], unsigned int img_width,
                  unsigned int img_height, unsigned int posX,
                  unsigned int posY);

   private:
    /**
     *  1.x
     *  2.y 
     *  3.(0, 0)
     *  4.y = x
     *  5.y = -x
     * 
    */
    void drawCirclePoint(unsigned int x0, unsigned int y0, unsigned int x, unsigned int y, RGB rgb = PIXEL_WHITE) {
        drawPixel(x0 + x, y0 + y, rgb);   // (x, y)
        
        // base (x, y)
        drawPixel(x0 + x, y0 - y, rgb);   // 1.(x, -y)
        drawPixel(x0 - x, y0 + y, rgb);   // 2.(-x, y)
        drawPixel(x0 - x, y0 - y, rgb);   // 3.(-x, -y)
        drawPixel(x0 + y, y0 + x, rgb);   // 4.(y, x)
        drawPixel(x0 - y, y0 + x, rgb);   // 5.(-y, x)
        
        // base (y, x)
        drawPixel(x0 + y, y0 - x, rgb);   // 1.(y, -x)

        // base (-y, x)
        drawPixel(x0 - y, y0 - x, rgb);   // 1.(-y, -x)
    }

};

void Graphics::drawRectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, RGB rgb) {
    drawXLine(y, x, x + w, rgb);
    drawYLine(x + w, y, y + h, rgb);
    drawXLine(y + h, x + w, x, rgb);
    drawYLine(x, y + h, y, rgb);
}

void Graphics::drawRectangle(Rectangle rect, RGB rgb) {
    drawRectangle(rect.x, rect.y, rect.w, rect.h, rgb);
}

void Graphics::drawTriangle(
    unsigned int x1, unsigned int y1,
    unsigned int x2, unsigned int y2,
    unsigned int x3, unsigned int y3, RGB rgb
) {
    drawLine(x1, y1, x2, y2, rgb);
    drawLine(x2, y2, x3, y3, rgb);
    drawLine(x3, y3, x1, y1, rgb);
}

void Graphics::drawTriangle(
    const TMATH::HomoCoordinates<3> &p1,
    const TMATH::HomoCoordinates<3> &p2,
    const TMATH::HomoCoordinates<3> &p3, RGB rgb
) {
    drawLine(p1, p2, rgb);
    drawLine(p2, p3, rgb);
    drawLine(p3, p1, rgb);
}

void Graphics::fillTriangle(int x1, int y1,  int x2,  int y2,  int x3,  int y3, RGB rgb)
{
    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) {
        swap(y1, y2);
        swap(x1, x2);
    }
    if (y1 > y3) {
        swap(y1, y3);
        swap(x1, x3);
    }
    if (y2 > y3) {
        swap(y2, y3);
        swap(x2, x3);
    }

    t1x = t2x = x1;
    y = y1;  // Starting points
    dx1 = (x2 - x1);
    if (dx1 < 0) {
        dx1 = -dx1;
        signx1 = -1;
    } else
        signx1 = 1;
    dy1 = (y2 - y1);
    dx2 = (x3 - x1);
    if (dx2 < 0) {
        dx2 = -dx2;
        signx2 = -1;
    } else
        signx2 = 1;
    dy2 = (y3 - y1);

    if (dy1 > dx1) {  // swap values
        swap(dx1, dy1);
        changed1 = true;
    }
    if (dy2 > dx2) {  // swap values
        swap(dy2, dx2);
        changed2 = true;
    }

    e2 = (dx2 >> 1);
    // Flat top, just process the second half
    if (y1 == y2) goto next;
    e1 = (dx1 >> 1);

    for (int i = 0; i < dx1;) {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x) {
            minx = t1x;
            maxx = t2x;
        } else {
            minx = t2x;
            maxx = t1x;
        }
        // process first line until y value is about to change
        while (i < dx1) {
            i++;
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1)
                    t1xp = signx1;  // t1x += signx1;
                else
                    goto next1;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
        }
        // Move line
    next1:
        // process second line until y value is about to change
        while (1) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2;  // t2x += signx2;
                else
                    goto next2;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next2:
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawXLine(y, minx, maxx, rgb);  // Draw line from min to max points
                                        // found on the y Now increase y
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y == y2) break;
    }
next:
    // Second half
    dx1 = (x3 - x2);
    if (dx1 < 0) {
        dx1 = -dx1;
        signx1 = -1;
    } else
        signx1 = 1;
    dy1 = (y3 - y2);
    t1x = x2;

    if (dy1 > dx1) {  // swap values
        swap(dy1, dx1);
        changed1 = true;
    } else
        changed1 = false;

    e1 = (dx1 >> 1);

    for (int i = 0; i <= dx1; i++) {
        t1xp = 0;
        t2xp = 0;
        if (t1x < t2x) {
            minx = t1x;
            maxx = t2x;
        } else {
            minx = t2x;
            maxx = t1x;
        }
        // process first line until y value is about to change
        while (i < dx1) {
            e1 += dy1;
            while (e1 >= dx1) {
                e1 -= dx1;
                if (changed1) {
                    t1xp = signx1;
                    break;
                }  // t1x += signx1;
                else
                    goto next3;
            }
            if (changed1)
                break;
            else
                t1x += signx1;
            if (i < dx1) i++;
        }
    next3:
        // process second line until y value is about to change
        while (t2x != x3) {
            e2 += dy2;
            while (e2 >= dx2) {
                e2 -= dx2;
                if (changed2)
                    t2xp = signx2;
                else
                    goto next4;
            }
            if (changed2)
                break;
            else
                t2x += signx2;
        }
    next4:
        if (minx > t1x) minx = t1x;
        if (minx > t2x) minx = t2x;
        if (maxx < t1x) maxx = t1x;
        if (maxx < t2x) maxx = t2x;
        drawXLine(y, minx, maxx, rgb);
        if (!changed1) t1x += signx1;
        t1x += t1xp;
        if (!changed2) t2x += signx2;
        t2x += t2xp;
        y += 1;
        if (y > y3) return;
    }
}

void Graphics::drawCircle(unsigned int x0, unsigned int y0, unsigned int r, RGB rgb) {
    unsigned int x = 0, y = r;  // init point
    while (x <= y) {
        drawCirclePoint(x0, y0, x, y, rgb);
        // F(x + 1, y - 0.5)
        double F = x * x + 2 * x + 1 + y * y - y + 0.25 - r * r;
        if (F <= 0) {
            x++;    // (x + 1, y)
        } else {
            y--;    // (x, y -1)
        }
    }
}

void Graphics::tuguiblt(unsigned char img[], unsigned int img_width,
                        unsigned int img_height, unsigned int posX,
                        unsigned int posY) {
    INTERFACE::gBaseInterfacePtr->tuguiblt(img, img_width, img_height, posX,
                                           posY);
}

/**
 * 
 * init global(static) var
 * 
*/
LinearEquation Base::__mLinearEquation;

};  /* TU end */



#endif

