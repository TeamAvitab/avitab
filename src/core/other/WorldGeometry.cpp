/* This file is part of the Avitab project. See the README and LICENSE for details. */

#include "WorldGeometry.h"
#include <cassert>

namespace world {

// Some of these functions use great circle geometry equations.
// See https://www.movable-type.co.uk/scripts/latlong.html for sources.

double Location::angularDistanceTo(const Location& targ) const
{
    auto& phi1 = ypos_rad;
    auto& lambda1 = xpos_rad;
    auto& phi2 = targ.ypos_rad;
    auto& lambda2 = targ.xpos_rad;
    double deltaPhi = phi2 - phi1;
    double deltaLambda = lambda2 - lambda1;

    double a = std::sin(deltaPhi / 2.0) * std::sin(deltaPhi / 2.0) +
        std::cos(phi1) * std::cos(phi2) *
        std::sin(deltaLambda / 2.0) * std::sin(deltaLambda / 2.0);
    return 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
}

double Location::surfaceDistanceTo(const Location& targ) const
{
    double ad = angularDistanceTo(targ);
    double RAD_TO_M = 1000.0f / KM_TO_RAD;
    return ad * RAD_TO_M;
}

bool Location::isContainedWithin(const Location& sw, const Location& ne) const
{
    // first eliminate above or below - no need to worry about wrapping for these
    if (ypos_rad < sw.ypos_rad) return false;
    if (ypos_rad > ne.ypos_rad) return false;
    // now deal with the simple case where the area doesn't span the -180/+180 discontinuity
    if (sw.xpos_rad < ne.xpos_rad) {
        // eliminate left or right of the area
        if (xpos_rad < sw.xpos_rad) return false;
        if (xpos_rad > ne.xpos_rad) return false;
        return true;
    }
    // if we get this far it's because the area spans the -180/+180 discontinuity
    if (xpos_rad > sw.xpos_rad) return true;
    if (xpos_rad < ne.xpos_rad) return true;
    return false;
}

Trajectory::Trajectory(const Location& here, const Location& targ)
:   Location(here)
{
    auto& phi1 = ypos_rad;
    auto& lambda1 = xpos_rad;
    auto& phi2 = targ.ypos_rad;
    auto& lambda2 = targ.xpos_rad;
    double deltaPhi = phi2 - phi1;
    double deltaLambda = lambda2 - lambda1;

    double Y = std::sin(deltaLambda) * std::cos(phi2);
    double X = std::cos(phi1) * std::sin(phi2) - std::sin(phi1) * std::cos(phi2) * std::cos(deltaLambda);

    hdg_rad = std::atan2(Y, X);
}

Trajectory Trajectory::greatCircleWaypoint(double angle) const
{
    if (fmod(angle, 2 * M_PI) == 0.0) return *this;

    auto& phi1 = ypos_rad;
    auto& lambda1 = xpos_rad;

    double phi2 = std::asin(std::sin(phi1) * std::cos(angle) + std::cos(phi1) * std::sin(angle) * std::cos(hdg_rad));
    double lambda2 = lambda1 + std::atan2(std::sin(hdg_rad) * std::sin(angle) * std::cos(phi1), std::cos(angle) - std::sin(phi1) * std::sin(phi2));

    // construct a trajectory, initially pointing back to this location ...
    Trajectory result(Location(phi2, lambda2), *this);
    // ... and then invert the direction
    result.hdg_rad += M_PI;
    while (result.hdg_rad >= M_PI) result.hdg_rad -= 2 * M_PI;
    return result;
}


// These tables were generated from instrumentation in earlier versions of Avitab sources.

static double worldToTileYXtests[][5] =
{
    { -80, -179, 1, 1.7754812040740084, 0.0055555555555555558 },
    { -80, 179, 1, 1.7754812040740084, 1.9944444444444445 },
    { 80, -179, 1, 0.22451879592599011, 0.0055555555555555558 },
    { 80, 179, 1, 0.22451879592599011, 1.9944444444444445 },
    { 47.789716, 13.000292, 5, 11.151539, 17.155582},
    { 47.789716, 13.000292, 6, 22.303078, 34.311163 },
    { 47.789716, 13.000292, 1, 0.696971, 1.072224 },
    { 47.789716, 13.000292, 3, 2.787885, 4.288895 },
    { 47.789716, 13.000292, 5, 11.151539, 17.155582 },
    { 47.789716, 13.000292, 4, 5.575770, 8.577791 },
    { 47.789716, 13.000292, 6, 22.303078, 34.311163 },
    { 47.804800, 12.996704, 11, 713.570769, 1097.936803 },
    { 47.782053, 13.011083, 11, 713.763387, 1098.018604 }
};

static double tileYXtoWorldTests[][5] =
{
    { 178.419922, 274.486328, 9, 47.791939, 12.998199 },
    { 4.041016, 7.041016, 4, 66.142743, -21.577148 },
    { 5.837891, 5.908203, 4, 43.675818, -47.065430 },
    { 39.966797, 20.998047, 6, -40.838749, -61.885986 },
    { 3.517578, 1.154297, 3, 21.207459, -128.056641 },
    { 3.548828, 4.982422, 3, 19.890723, 44.208984 },
    { 6.642578, 5.923828, 3, -75.693931, 86.572266 },
    { 4.541016, 4.712891, 3, -23.644524, 32.080078 },
    { 10.107422, 10.708984, 4, -42.779275, 60.952148 },
    { 30.873047, 33.623047, 6, 6.326218, 9.129639 },
    { -0.521484, 2.529297, 1, 89.037801, -84.726562 },
    { -0.466797, 2.705078, 1, 88.857450, -53.085938 },
    { -0.580078, -0.904297, 1, 89.199568, 17.226562 },
    { 0.939453, 0.017578, 1, 10.833306, -176.835938 },
    { 1.849609, 0.021484, 2, 13.410994, -178.066406 },
    { 84.189453, 125.837891, 7, -49.271389, 173.919067 },
    { 81.216797, 122.486328, 7, -43.512705, 164.492798 },
    { 165.423828, 247.580078, 8, -46.485156, 168.159485 },
    { 10.044922, 16.904297, 4, -41.738528, -159.653320 },
    { 5.982422, 13.943359, 4, 41.277806, 133.725586 },
    { 5.826172, 13.947266, 4, 43.866218, 133.813477 },
    { 7.072266, 12.408203, 4, 20.427013, 99.184570 },
    { 71.310547, 232.998047, 8, 62.064020, 147.653503 },
    { 5.802734, -0.681641, 4, 44.245199, 164.663086 },
    { 5.787109, -1.142578, 4, 44.496505, 154.291992 },
    { 5.728516, 1.787109, 4, 45.429299, -139.790039 },
    { 536.498047, 28.998047, 11, 64.736788, -174.902687 },
    { 4.189453, 0.224609, 4, 64.755390, -174.946289 },
    { 14.748047, 11.591797, 6, 69.166466, -114.796143 },
    { 66.654297, 38.716797, 8, 64.980521, -125.554504 },
    { 3.962891, 1.599609, 3, 1.669686, -108.017578 },
    { 2.103516, 2.326172, 1, -86.423971, -121.289062 },
    { 1.939453, -0.732422, 1, -84.016022, 48.164062 },
    { -1.228516, -0.767578, 1, 89.895620, 41.835938 },
    { -1.326172, -0.775391, 1, 89.923197, 40.429688 },
    { -1.419922, 2.259766, 1, 89.942790, -133.242188 },
    { 1.642578, -0.802734, 1, -74.867889, 35.507812 },
    { 1.642578, 2.259766, 1, -74.867889, -133.242188 },
    { 1.642578, 2.259766, 1, -74.867889, -133.242188 },
    { 2.435547, 4.310547, 4, 77.166927, -83.012695 },
    { 1.818359, 7.365234, 4, 79.912854, -14.282227 },
    { 2.005859, 13.208984, 5, 82.667877, -31.398926 },
    { -0.021484, -0.708984, 1, 85.373767, 52.382812 },
    { 3.205078, -0.728516, 1, -89.887644, 48.867188 },
    { 3.412109, 2.267578, 1, -89.941369, -131.835938 },
    { 2.052734, 0.712891, 1, -85.805958, -51.679688 },
    { 3.626953, -0.814453, 1, -89.970146, 33.398438 },
    { 2.033203, 0.724609, 1, -85.540816, -49.570312 },
    { 0.501953, -0.806641, 1, 66.372755, 34.804688 },
    { 2.017578, 0.728516, 1, -85.316708, -48.867188 },
    { 3.548828, 2.259766, 1, -89.961841, -133.242188 },
    { 63.068359, 19.962891, 6, -84.577818, -67.708740 },
    { 46.134766, 70.150391, 7, 44.820812, 17.297974 },
    { 178.419922, 274.486328, 9, 47.791939, 12.998199 },
    { 712.162109, 1099.486328, 11, 47.970847, 13.269081 }
};

static double worldToMercatorTests[][4] = {
    { 47.789716, 13.000292, 54.545183, 13.000292 },
};

static double worldFromMercatorTests[][4] = {
    {54.020247, -122.257037, 47.435838, -122.257037},
    {54.117604, -122.228673, 47.501651, -122.228673},
    {53.964373, -122.344432, 47.398031, -122.344432},
    {54.117219, -122.382443, 47.501391, -122.382443},
    {-52.550543, 168.351721, -46.432309, 168.351721},
    {-52.555410, 168.279007, -46.435663, 168.279007},
    {-52.572820, 168.281431, -46.447660, 168.281431},
    {-52.503650, 168.354052, -46.399980, 168.354052},
    {-52.515852, 168.329686, -46.408394, 168.329686},
    {-52.540241, 168.329257, -46.425208, 168.329257},
    {-52.510359, 168.336146, -46.404606, 168.336146},
    {-52.536171, 168.310437, -46.422402, 168.310437},
    {-52.510359, 168.310437, -46.404606, 168.310437},
    {-52.536171, 168.310437, -46.422402, 168.310437},
    {-52.512245, 168.325335, -46.405907, 168.325335},
    {-52.538355, 168.324808, -46.423908, 168.324808},
    {-52.913269, 169.083606, -46.681730, 169.083606},
    {-53.077154, 167.989472, -46.794046, 167.989472},
    {-53.102806, 169.093552, -46.811605, 169.093552},
    {-53.293440, 167.867236, -46.941917, 167.867236},
    {-52.181143, 168.975235, -46.177120, 168.975235},
    {-53.293440, 167.867236, -46.941917, 167.867236},
    {-53.500213, 168.833184, -47.082903, 168.833184},
    {38.737597, 140.868225, 36.084463, 140.868225},
};

static double worldToEPSG3857Tests[][4] = {
    {47.789716, 13.000292, 6071942.051257, 1447185.934673},
};

static double worldFromEPSG3857Tests[][4] = {
    { 6074514.845874, 1451932.399545, 47.805241, 13.042931 },
    { -3170282.111099, 17047042.563042, -27.374046, 153.136189 },
    { -3176625.271825, 17047186.931801, -27.424636, 153.137486 },
    { -3157709.517856, 17026973.674829, -27.273706, 152.955907 },
    { -3184452.584185, 17034018.619592, -27.487030, 153.019193 },
    { 6011708.584247, -13611247.224916, 47.424913, -122.271914 },
    { 6007577.498793, -13622434.559082, 47.399800, -122.372412 },
    { 6006923.112671, -13620126.434986, 47.395821, -122.351677 },
    { 6009989.839759, -13620360.844479, 47.414466, -122.353783 },
    { 6023602.099583, -13624453.459272, 47.497147, -122.390548 },
    { 6008861.713230, -13622551.465220, 47.407608, -122.373462 },
    { 6008265.414232, -13604745.778381, 47.403983, -122.213511 },
    { 6023137.653312, -13604725.147289, 47.494328, -122.213325 },
    { 6008168.305131, -13619666.459405, 47.403392, -122.347545 },
    { 6022354.871188, -13607223.827237, 47.489577, -122.235771 },
    { 6005949.207418, -13623067.126001, 47.389898, -122.378094 },
    { 6016815.885886, -13619035.977938, 47.455944, -122.341882 },
    { 7129834.462471, 1189639.504733, 53.786549, 10.686713 },
    { 7129866.605390, 1189629.032648, 53.786720, 10.686619 },
    { 7132450.315187, 1192066.200532, 53.800430, 10.708513 },
    { 7133099.237227, 1190448.885301, 53.803872, 10.693984 },
    { 7129583.090890, 1190939.411426, 53.785215, 10.698391 },
    { 7134010.099581, 1189497.102684, 53.808704, 10.685434 },
    { 7133526.489763, 1191035.651594, 53.806139, 10.699255 },
    { 7132710.182931, 1191905.954654, 53.801808, 10.707073 },
    { 7131881.051419, 1190419.710784, 53.797409, 10.693722 },
    { 7132224.305295, 1190761.704184, 53.799231, 10.696794 },
    { 7133990.421183, 1190694.420071, 53.808600, 10.696190 },
    { 7129959.815491, 1101000.054887, 53.787214, 9.890452 },
    { 7191732.340470, 1225500.703021, 54.113769, 11.008860 },
    { 7090961.568985, 1115832.322609, 53.579735, 10.023692 },
    { 7133775.898057, 1121425.001830, 53.807462, 10.073932 },
    { 7191962.469948, 1121777.137443, 54.114981, 10.077095 },
    { 7170085.579168, 1126139.139469, 53.999627, 10.116280 },
    { 7079340.708315, 1198600.230355, 53.517712, 10.767209 },
    { 7093371.229222, 1125730.801180, 53.592585, 10.112612 },
    { 7151557.801113, 1126082.936793, 53.901681, 10.115775 },
    { 7141513.934165, 1176815.526889, 53.848489, 10.571514 },
    { 7179580.621600, 1214387.037797, 54.049732, 10.909024 },
};


inline bool almostEqual(double a, double b) {
    return std::fabs((a - b) / (a + b)) < 0.000006;
}

void RunGeometryUnitTests() {
#ifndef NDEBUG
    size_t n = sizeof(worldToTileYXtests) / sizeof(double) / 5;
    double *p = worldToTileYXtests[0];
    for (size_t i = 0; i < n; ++i) {
        double lat = *p++;
        double lon = *p++;
        int zoom   = (int)*p++;
        double tileY = *p++;
        double tileX = *p++;

        // world to tile
        Location wloc = Location::fromGCS(lat, lon);
        auto tyx      = wloc.toTileYX(zoom);
        assert(almostEqual(tyx.first, tileY));
        assert(almostEqual(tyx.second, tileX));

        // and back to world
        auto wloc2 = Location::fromTileYX(tileY, tileX, zoom);
        assert(almostEqual(wloc2.latDegrees(), lat));
        assert(almostEqual(wloc2.lonDegrees(), lon));
    }

    n  = sizeof(tileYXtoWorldTests) / sizeof(double) / 5;
    p = tileYXtoWorldTests[0];
    for (size_t i = 0; i < n; ++i) {
        double tileY = *p++;
        double tileX = *p++;
        int zoom     = (int)*p++;
        double lat   = *p++;
        double lon   = *p++;

        // tileYX to world
        auto wloc    = Location::fromTileYX(tileY, tileX, zoom);
        double wlat  = wloc.latDegrees();
        double wlon  = wloc.lonDegrees();
        assert(almostEqual(wlat, lat));
        assert(almostEqual(wlon, lon));

        // and back to tileYX
        auto tyx = wloc.toTileYX(zoom);
        assert(almostEqual(tyx.first, tileY));
        double xmax = std::pow(2.0, zoom);
        while (tileX < 0) tileX += xmax;
        while (tileX >= xmax) tileX -= xmax;
        assert(almostEqual(tyx.second, tileX));
    }

    n  = sizeof(worldToMercatorTests) / sizeof(double) / 4;
    p = worldToMercatorTests[0];
    for (size_t i = 0; i < n; ++i) {
        double lat    = *p++;
        double lon    = *p++;
        double mercLat  = *p++;
        double mercLon  = *p++;

        // world to mercator
        Location wloc = Location::fromGCS(lat, lon);
        auto mlatlon      = wloc.toMercator();
        assert(almostEqual(mlatlon.first, mercLat));
        assert(almostEqual(mlatlon.second, mercLon));

        // and back to world
        auto wloc2 = Location::fromMercator(mlatlon.first, mlatlon.second);
        assert(almostEqual(wloc2.latDegrees(), lat));
        assert(almostEqual(wloc2.lonDegrees(), lon));
    }

    n  = sizeof(worldFromMercatorTests) / sizeof(double) / 4;
    p = worldFromMercatorTests[0];
    for (size_t i = 0; i < n; ++i) {
        double mercLat  = *p++;
        double mercLon  = *p++;
        double lat    = *p++;
        double lon    = *p++;

        // mercator to world
        auto wloc = Location::fromMercator(mercLat, mercLon);
        assert(almostEqual(wloc.latDegrees(), lat));
        assert(almostEqual(wloc.lonDegrees(), lon));

        // and back to mercator
        auto mlatlon      = wloc.toMercator();
        assert(almostEqual(mlatlon.first, mercLat));
        assert(almostEqual(mlatlon.second, mercLon));
    }

    n  = sizeof(worldToEPSG3857Tests) / sizeof(double) / 4;
    p = worldToEPSG3857Tests[0];
    for (size_t i = 0; i < n; ++i) {
        double lat    = *p++;
        double lon    = *p++;
        double y  = *p++;
        double x  = *p++;

        // world to ESPG3857
        Location wloc = Location::fromGCS(lat, lon);
        auto yx      = wloc.toEPSG3857();
        assert(almostEqual(yx.first, y));
        assert(almostEqual(yx.second, x));

        // and back to world
        auto wloc2 = Location::fromEPSG3857(yx.first, yx.second);
        assert(almostEqual(wloc2.latDegrees(), lat));
        assert(almostEqual(wloc2.lonDegrees(), lon));
    }

    n  = sizeof(worldFromEPSG3857Tests) / sizeof(double) / 4;
    p = worldFromEPSG3857Tests[0];
    for (size_t i = 0; i < n; ++i) {
        double y  = *p++;
        double x  = *p++;
        double lat    = *p++;
        double lon    = *p++;

        // ESPG3857 to world
        auto wloc = Location::fromEPSG3857(y, x);
        assert(almostEqual(wloc.latDegrees(), lat));
        assert(almostEqual(wloc.lonDegrees(), lon));

        // and back to mercator
        auto yx      = wloc.toEPSG3857();
        assert(almostEqual(yx.first, y));
        assert(almostEqual(yx.second, x));
    }

#endif
}

}
