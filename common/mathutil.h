#ifndef VIDREVOLT_MATHUTIL_H_
#define VIDREVOLT_MATHUTIL_H_

namespace vidrevolt {
    namespace mathutil {
        float remap(float value, float in_min, float in_max, float out_min, float out_max);

        struct DrawInfo {
            int x0, x1, y0, y1;
            static DrawInfo scaleCenter(float w, float h, float dest_w, float dest_h);
        };
    }
}

#endif
