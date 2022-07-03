#pragma once

// --- M-Series --- //
#define DEFAULT_M_STT STT_M32X

#ifdef __cplusplus
extern "C" {
#endif

//=================================================
// Return the false accept rate (FAR) for the given
// threshold. The FAR is returned as a percentage,
// i.e. values range from 0% to 100%.
//=================================================
// --- V-Series --- //
float GetFARForFullSized(unsigned int Threshold);

float GetFARForCropped(unsigned int Threshold);

float GetFRRForFullSized(unsigned int Threshold);

float GetFRRForCropped(unsigned int Threshold);

// --- M-Series --- //
float GetFARForMX2X(unsigned int Threshold);

float GetFARForVenusCroppedMX2X(unsigned int Threshold);

float GetFRRForMX2X(unsigned int Threshold);

float GetFRRForVenusCroppedMX2X(unsigned int Threshold);

//====================================================
// Return the threshold value which corresponds to the
// given false accept rate (FAR). The FAR is specified
// as a percentage, i.e. input values can range from
// 0% to 100%.
//====================================================
// --- V-Series --- //
float GetThresholdFromFARForFullSized(float FAR);

// --- M-Series --- //
float GetThresholdFromFARForMX2X(float FAR);

//====================================================================================================================
// --- V-Series --- //
const unsigned int FullSized_ThresholdTable[] = {
     8998,  9248,  9498,  9998, 10848, 11448, 12398, 12598, 12948, 13098,
    13298, 13448, 13948, 14148, 14298, 14448, 14648, 14798, 14948, 15098,
    15298, 15448, 15598, 15898, 16048, 16248, 16398, 16548, 16698, 16848,
    17148, 17298, 17448, 17598, 17748, 17898, 18048, 18198, 18348, 18498,
    18798, 19098, 19248, 19548, 19648, 19798, 19948, 20098, 20248, 20398,
    20498, 20648, 20798, 20948, 21048, 21198, 21348, 21498, 21598, 21748,
    21898, 22048, 22198, 22298, 22448, 22598, 22748, 22898, 22998, 23148,
    23298, 23448, 23548, 23698, 23798, 23948, 24098, 24248, 24348, 24498,
    24648, 24798, 24898, 25048, 25198, 25348, 25448, 25598, 25748, 25898,
    26048, 26148, 26298, 26448, 26598, 26748, 26848, 26998, 27148, 27298,
    27398, 27548, 27698, 27848, 27998, 28098, 28248, 28398, 28548, 28648,
    28798, 28948, 29098, 29248, 29348, 29498, 29648, 29798, 29948, 30048,
    30198, 30348, 30498, 30598, 30748, 31298, 31898, 31998, 32848, 33148,
    34798};

#define FullSized_Table_Size        (sizeof(FullSized_ThresholdTable) / sizeof(FullSized_ThresholdTable[0]))

const float FullSized_FARTable[] = {
    88.920790f, 84.779450f, 80.163932f, 70.290855f, 51.599770f, 40.065883f, 25.688914f, 23.456008f, 19.530595f, 17.813927f,
    16.244112f, 14.814597f, 11.226543f, 10.237599f,  9.333737f,  8.507414f,  7.754417f,  7.071736f,  6.446283f,  5.871666f,
     5.353897f,  4.877259f,  4.445323f,  3.691506f,  3.359210f,  3.060320f,  2.787554f,  2.539060f,  2.311536f,  2.102951f,
     1.741921f,  1.585063f,  1.441461f,  1.310765f,  1.191066f,  1.081451f,  0.985023f,  0.896068f,  0.814517f,  0.740480f,
     0.611436f,  0.506673f,  0.460040f,  0.380330f,  0.345713f,  0.313507f,  0.284642f,  0.258649f,  0.234867f,  0.213267f,
     0.194387f,  0.176438f,  0.159920f,  0.145533f,  0.132956f,  0.120320f,  0.109375f,  0.099400f,  0.090645f,  0.081891f,
     0.074177f,  0.067324f,  0.061061f,  0.055298f,  0.050015f,  0.045433f,  0.041531f,  0.037629f,  0.034327f,  0.031096f,
     0.028164f,  0.025363f,  0.023172f,  0.021071f,  0.019130f,  0.017179f,  0.015568f,  0.014137f,  0.012946f,  0.011716f,
     0.010685f,  0.009735f,  0.008804f,  0.008034f,  0.007304f,  0.006553f,  0.005853f,  0.005273f,  0.004872f,  0.004492f,
     0.004062f,  0.003642f,  0.003232f,  0.002851f,     0.002561f,  0.002251f,  0.002091f,  0.001861f,  0.001681f,  0.001561f,
     0.001461f,  0.001311f,  0.001111f,  0.000980f,  0.000850f,  0.000750f,  0.000700f,  0.000620f,  0.000560f,  0.000520f,
     0.000450f,  0.000390f,  0.000330f,  0.000300f,  0.000260f,  0.000250f,  0.000240f,  0.000220f,  0.000200f,  0.000150f,
     0.000140f,  0.000110f,  0.000100f,  0.000080f,  0.000070f,  0.000060f,  0.000040f,  0.000030f,  0.000020f,  0.000010f,
     0.000000f};


const float FullSized_FRRTable[] = {
    0.000000f, 0.005000f, 0.010000f, 0.020000f, 0.030000f, 0.035000f, 0.040000f, 0.050000f, 0.060000f, 0.065000f,
    0.075000f, 0.095000f, 0.100000f, 0.105000f, 0.110000f, 0.120000f, 0.125000f, 0.130000f, 0.135000f, 0.140000f,
    0.150000f, 0.160000f, 0.165000f, 0.170000f, 0.175000f, 0.185000f, 0.190000f, 0.195000f, 0.200000f, 0.215000f,
    0.225000f, 0.245000f, 0.250000f, 0.260000f, 0.270000f, 0.275000f, 0.290000f, 0.300000f, 0.310000f, 0.315000f,
    0.330000f, 0.340000f, 0.345000f, 0.355000f, 0.375000f, 0.380000f, 0.405000f, 0.450000f, 0.465000f, 0.480000f,
    0.495000f, 0.500000f, 0.520000f, 0.535000f, 0.540000f, 0.555000f, 0.575000f, 0.595000f, 0.605000f, 0.620000f,
    0.635000f, 0.670000f, 0.680000f, 0.685000f, 0.700000f, 0.720000f, 0.735000f, 0.750000f, 0.765000f, 0.810000f,
    0.840000f, 0.855000f, 0.880000f, 0.910000f, 0.935000f, 0.970000f, 0.990000f, 1.015000f, 1.035000f, 1.060000f,
    1.080000f, 1.095000f, 1.105000f, 1.135000f, 1.170000f, 1.215000f, 1.240000f, 1.275000f, 1.295000f, 1.330000f,
    1.350000f, 1.375000f, 1.415000f, 1.440000f, 1.485000f, 1.515000f, 1.555000f, 1.585000f, 1.620000f, 1.665000f,
    1.700000f, 1.745000f, 1.810000f, 1.850000f, 1.900000f, 1.930000f, 1.975000f, 2.020000f, 2.060000f, 2.095000f,
    2.150000f, 2.185000f, 2.230000f, 2.265000f, 2.305000f, 2.335000f, 2.385000f, 2.425000f, 2.465000f, 2.515000f,
    2.550000f, 2.620000f, 2.695000f, 2.720000f, 2.775000f, 2.955000f, 3.130000f, 3.195000f, 3.470000f, 3.555000f,
    4.195000f};



const unsigned int Cropped_ThresholdTable[] = {
    10377, 10727, 11077, 11377, 11677, 11977, 12277, 12527, 12777, 13027,
    13277, 13727, 14177, 14427, 14627, 15077, 15277, 15477, 15677, 15877,
    16077, 16277, 16477, 16677, 16877, 17077, 17277, 17477, 17677, 17877,
    18077, 18277, 18477, 18677, 18877, 19027, 19227, 19427, 19627, 19827,
    19977, 20177, 20377, 20577, 20727, 20927, 21127, 21477, 21677, 21827,
    22027, 22227, 22427, 22577, 22777, 22977, 23127, 23327, 23477, 23677,
    23877, 24027, 24227, 24377, 24577, 24777, 24927, 25127, 25277, 25477,
    25677, 25877, 26027, 26227, 26427, 26577, 26777, 26977, 27127, 27327,
    27477, 27627, 27827, 27977, 28177, 28327, 28477, 28677, 28827, 29027,
    29177, 29377, 29527, 29677, 29877, 30027, 30227, 30377, 30577, 30727,
    30877, 31077, 31227, 31427, 31577, 31777, 31927, 32127, 32277, 32427,
    32627, 32777, 32977, 33127, 33327, 33477, 33627, 34177, 34327, 34527,
    35027, 35177, 35527, 36077, 37427};

#define Cropped_Table_Size        (sizeof(Cropped_ThresholdTable) / sizeof(Cropped_ThresholdTable[0]))
const float Cropped_FARTable[] = {
    84.518509f, 78.830075f, 72.844752f, 66.840850f, 61.061801f, 55.612756f, 50.525083f, 45.827094f, 41.509675f, 37.564062f,
    33.951716f, 27.673847f, 22.477219f, 20.244442f, 18.220020f, 14.745353f, 13.258049f, 11.915628f, 10.708204f,  9.615238f,
     8.636158f,  7.750395f,  6.957839f,  6.246113f,  5.606163f,  5.028984f,  4.509295f,  4.041911f,  3.621841f,  3.248004f,
     2.914067f, 2.609275f, 2.338289f, 2.091246f, 1.872306f, 1.675768f, 1.501581f, 1.344582f, 1.203222f, 1.075318f,
     0.963212f, 0.862661f, 0.770765f, 0.687364f, 0.615478f, 0.549485f, 0.490915f, 0.391436f, 0.350015f, 0.312436f,
     0.279320f, 0.248914f, 0.221751f, 0.197479f, 0.175688f, 0.156678f, 0.139990f, 0.125213f, 0.112446f, 0.099650f,
     0.089185f, 0.079770f, 0.071066f, 0.063562f, 0.056648f, 0.050715f, 0.045363f, 0.040710f, 0.036278f, 0.032296f,
     0.028584f, 0.025453f, 0.022751f, 0.020280f, 0.018099f, 0.016078f, 0.014387f, 0.012626f, 0.011396f, 0.010135f,
     0.009005f, 0.007964f, 0.007094f, 0.006323f, 0.005683f, 0.005103f, 0.004482f, 0.003972f, 0.003462f, 0.003092f,
     0.002811f, 0.002511f, 0.002181f, 0.001951f, 0.001781f, 0.001641f, 0.001481f, 0.001301f, 0.001151f, 0.001031f,
     0.000900f, 0.000750f, 0.000660f, 0.000560f, 0.000520f, 0.000420f, 0.000410f, 0.000360f, 0.000350f, 0.000290f,
     0.000230f, 0.000210f, 0.000190f, 0.000150f, 0.000130f, 0.000110f, 0.000100f, 0.000090f, 0.000080f, 0.000060f,
     0.000040f, 0.000030f, 0.000020f, 0.000010f, 0.000000f};

const float Cropped_FRRTable[] = {
    0.000000f, 0.005000f, 0.010000f, 0.015000f, 0.020000f, 0.030000f, 0.045000f, 0.060000f, 0.070000f, 0.075000f,
    0.085000f, 0.095000f, 0.115000f, 0.135000f, 0.140000f, 0.155000f, 0.185000f, 0.205000f, 0.220000f, 0.235000f,
    0.255000f, 0.270000f, 0.295000f, 0.300000f, 0.315000f, 0.330000f, 0.345000f, 0.360000f, 0.375000f, 0.405000f,
    0.415000f, 0.420000f, 0.455000f, 0.480000f, 0.500000f, 0.515000f, 0.530000f, 0.540000f, 0.565000f, 0.585000f,
    0.640000f, 0.655000f, 0.665000f, 0.695000f, 0.710000f, 0.720000f, 0.750000f, 0.765000f, 0.790000f, 0.830000f,
    0.850000f, 0.885000f, 0.910000f, 0.925000f, 0.950000f, 1.015000f, 1.050000f, 1.090000f, 1.125000f, 1.160000f,
    1.200000f, 1.220000f, 1.240000f, 1.275000f, 1.335000f, 1.360000f, 1.405000f, 1.450000f, 1.475000f, 1.505000f,
    1.545000f, 1.580000f, 1.610000f, 1.640000f, 1.685000f, 1.735000f, 1.780000f, 1.850000f, 1.890000f, 1.925000f,
    1.975000f, 2.020000f, 2.060000f, 2.105000f, 2.145000f, 2.200000f, 2.245000f, 2.275000f, 2.320000f, 2.370000f,
    2.435000f, 2.480000f, 2.510000f, 2.555000f, 2.625000f, 2.690000f, 2.735000f, 2.785000f, 2.825000f, 2.870000f,
    2.935000f, 2.975000f, 3.025000f, 3.110000f, 3.185000f, 3.240000f, 3.310000f, 3.360000f, 3.415000f, 3.475000f,
    3.540000f, 3.595000f, 3.635000f, 3.695000f, 3.760000f, 3.840000f, 3.945000f, 4.270000f, 4.350000f, 4.430000f,
    4.685000f, 4.770000f, 4.940000f, 5.200000f, 5.920000f};



//====================================================================================================================
// --- M-Series --- //

const unsigned int MX2X_ThresholdTable[] = {
    6310, 8815, 9140, 9360,
    9530, 9675, 9800, 9910,
    10015, 10110, 10200, 10285,
    10365, 10440, 10515, 10585,
    10650, 10720, 10785, 10850,
    10910, 10970, 11030, 11090,
    11150, 11205, 11265, 11320,
    11375, 11430, 11485, 11540,
    11595, 11650, 11700, 11755,
    11810, 11860, 11915, 11970,
    12025, 12075, 12130, 12185,
    12235, 12290, 12345, 12400,
    12455, 12510, 12565, 12625,
    12680, 12735, 12795, 12850,
    12910, 12970, 13030, 13090,
    13155, 13215, 13280, 13345,
    13410, 13480, 13550, 13620,
    13690, 13765, 13840, 13915,
    13995, 14075, 14155, 14245,
    14330, 14420, 14515, 14615,
    14715, 14820, 14930, 15045,
    15170, 15295, 15430, 15580,
    15735, 15900, 16080, 16175,
    16280, 16385, 16495, 16615,
    16740, 16875, 17020, 17175,
    17340, 17435, 17530, 17630,
    17735, 17850, 17965, 18095,
    18235, 18385, 18550, 18730,
    18930, 19160, 19420, 19725,
    20250, 20430, 20655, 20900,
    21180, 21525, 21970, 22590,
    23605, 23750, 23825, 23910,
    24005, 24095, 24195, 24290,
    24410, 24535, 24670, 24820,
    24980, 25185, 25390, 25525,
    25530, 25535, 25540, 25545,
    25610, 25855, 26225, 26640,
    27265, 27390, 27630, 27845,
    28095, 28325, 28550, 29040,
    29380, 30475
};

#define MX2X_Table_Size        (sizeof(MX2X_ThresholdTable) / sizeof(MX2X_ThresholdTable[0]))

const float MX2X_FARTable[] = {
    99.999971f, 98.995097f, 98.003835f, 96.995552f,
    95.998635f, 94.983123f, 93.988101f, 93.014151f,
    91.996395f, 90.984177f, 89.981636f, 88.978336f,
    88.003337f, 87.006303f, 85.976423f, 84.980701f,
    84.034702f, 82.994825f, 81.997442f, 80.967241f,
    80.009962f, 79.033593f, 78.023298f, 77.013966f,
    75.980238f, 75.024766f, 73.964750f, 72.992607f,
    71.998750f, 70.995188f, 69.996552f, 68.971364f,
    67.966695f, 66.945121f, 66.012004f, 64.995822f,
    63.971654f, 63.036264f, 62.017225f, 60.989910f,
    59.957902f, 59.018140f, 57.987356f, 56.958350f,
    56.024592f, 55.004359f, 53.994443f, 52.998051f,
    51.999386f, 51.009697f, 50.028781f, 48.958943f,
    47.988694f, 47.025207f, 45.983523f, 45.030849f,
    44.010791f, 43.007229f, 42.022233f, 41.040617f,
    39.989432f, 39.028655f, 38.010492f, 37.018442f,
    36.034175f, 34.987215f, 33.965758f, 32.965286f,
    31.978279f, 30.969237f, 29.967483f, 28.990880f,
    27.975660f, 26.988187f, 26.020007f, 24.972931f,
    23.994085f, 23.010196f, 22.003457f, 20.982729f,
    19.999773f, 19.010375f, 18.012818f, 17.013657f,
    15.991821f, 15.009243f, 14.010928f, 12.986323f,
    11.995672f, 10.994093f, 9.998167f, 9.511702f,
    8.992681f, 8.499367f, 8.006169f, 7.498195f,
    7.003482f, 6.502240f, 6.004817f, 5.495268f,
    5.005160f, 4.745912f, 4.492202f, 4.246827f,
    4.001540f, 3.748238f, 3.505107f, 3.253233f,
    2.999669f, 2.746192f, 2.499564f, 2.249730f,
    1.999808f, 1.748051f, 1.497925f, 1.249141f,
    0.908810f, 0.811115f, 0.709340f, 0.609896f,
    0.510248f, 0.409201f, 0.310369f, 0.210138f,
    0.110169f, 0.100027f, 0.095043f, 0.090001f,
    0.084871f, 0.080004f, 0.075020f, 0.070007f,
    0.065111f, 0.060010f, 0.054997f, 0.050072f,
    0.044971f, 0.039987f, 0.035004f, 0.032002f,
    0.031943f, 0.031914f, 0.031827f, 0.031710f,
    0.029991f, 0.024948f, 0.019965f, 0.015039f,
    0.009997f, 0.008977f, 0.008015f, 0.006995f,
    0.006033f, 0.005013f, 0.003993f, 0.003002f,
    0.002011f, 0.000991f

};

const float MX2X_FRRTable[] = {
    0.001000f, 0.003059f, 0.005345f, 0.006045f,
    0.006586f, 0.008053f, 0.009544f, 0.010857f,
    0.013028f, 0.015748f, 0.018325f, 0.020759f,
    0.023031f, 0.024911f, 0.026790f, 0.028543f,
    0.030172f, 0.031926f, 0.033763f, 0.035934f,
    0.037938f, 0.039943f, 0.041947f, 0.043951f,
    0.045956f, 0.048928f, 0.052446f, 0.055670f,
    0.058895f, 0.062120f, 0.065344f, 0.068719f,
    0.072218f, 0.075718f, 0.078899f, 0.082399f,
    0.085898f, 0.090193f, 0.094918f, 0.099642f,
    0.104366f, 0.108661f, 0.114022f, 0.120496f,
    0.126382f, 0.132856f, 0.139330f, 0.145805f,
    0.151921f, 0.157958f, 0.163994f, 0.170580f,
    0.176617f, 0.184538f, 0.195648f, 0.205832f,
    0.216941f, 0.227647f, 0.236332f, 0.245017f,
    0.254426f, 0.263111f, 0.273481f, 0.286013f,
    0.298545f, 0.312040f, 0.325841f, 0.339871f,
    0.353901f, 0.369086f, 0.386409f, 0.403732f,
    0.422209f, 0.441059f, 0.459995f, 0.481298f,
    0.504223f, 0.529134f, 0.555571f, 0.586113f,
    0.616521f, 0.645784f, 0.676440f, 0.710274f,
    0.747736f, 0.784670f, 0.824262f, 0.868070f,
    0.917204f, 0.972675f, 1.029024f, 1.062300f,
    1.100883f, 1.143224f, 1.189155f, 1.231095f,
    1.273567f, 1.322346f, 1.374851f, 1.430434f,
    1.490670f, 1.522023f, 1.552489f, 1.586733f,
    1.631830f, 1.679599f, 1.724326f, 1.779754f,
    1.840134f, 1.894536f, 1.946628f, 2.008661f,
    2.090647f, 2.179909f, 2.273061f, 2.388547f,
    2.593009f, 2.663445f, 2.749236f, 2.821226f,
    2.935815f, 3.058089f, 3.211477f, 3.420472f,
    3.798210f, 3.855404f, 3.884037f, 3.914221f,
    3.946182f, 3.975376f, 4.005917f, 4.036722f,
    4.080530f, 4.117251f, 4.150494f, 4.200501f,
    4.258077f, 4.332260f, 4.412217f, 4.467812f,
    4.469928f, 4.472043f, 4.474159f, 4.476275f,
    4.503778f, 4.603102f, 4.702754f, 4.869458f,
    5.128256f, 5.167149f, 5.250298f, 5.327869f,
    5.408781f, 5.501300f, 5.588666f, 5.776617f,
    5.895872f, 6.314448f
};
const unsigned int VenusCroppedMX2X_ThresholdTable[] = {
    6464, 8589, 8919, 9144,
    9319, 9464, 9594, 9709,
    9814, 9914, 10004, 10089,
    10174, 10249, 10329, 10399,
    10469, 10539, 10604, 10669,
    10729, 10794, 10854, 10914,
    10974, 11029, 11089, 11144,
    11199, 11254, 11314, 11364,
    11419, 11474, 11529, 11584,
    11639, 11689, 11744, 11799,
    11854, 11909, 11959, 12014,
    12069, 12124, 12179, 12234,
    12289, 12349, 12404, 12459,
    12514, 12574, 12634, 12689,
    12749, 12814, 12874, 12934,
    12999, 13064, 13129, 13194,
    13264, 13329, 13399, 13474,
    13544, 13619, 13694, 13774,
    13854, 13934, 14024, 14109,
    14199, 14294, 14389, 14489,
    14589, 14699, 14814, 14929,
    15054, 15184, 15324, 15469,
    15629, 15794, 15974, 16074,
    16179, 16289, 16404, 16524,
    16659, 16794, 16939, 17104,
    17274, 17369, 17469, 17574,
    17689, 17804, 17919, 18054,
    18189, 18339, 18514, 18699,
    18899, 19134, 19389, 19704,
    20229, 20434, 20649, 20914,
    21219, 21594, 22024, 22644,
    23719, 23894, 23969, 24044,
    24124, 24234, 24354, 24439,
    24549, 24639, 24779, 24909,
    25074, 25224, 25459, 25744,
    26049, 26514, 26939, 27529,
    27649, 27804, 27964, 28194,
    28394, 28844, 29274, 29659,
    30309, 30369, 30414, 30674,
    32084, 32304 };

#define VenusCroppedMX2X_Table_Size        (sizeof(VenusCroppedMX2X_ThresholdTable) / sizeof(VenusCroppedMX2X_ThresholdTable[0]))

const float VenusCroppedMX2X_FARTable[] = {
    99.999819f, 99.004652f, 98.001327f, 96.996914f,
    95.996309f, 95.004768f, 93.996729f, 93.012078f,
    92.024889f, 90.992737f, 89.997027f, 89.017271f,
    87.968439f, 87.018235f, 85.976656f, 85.003245f,
    83.984328f, 82.965774f, 81.979310f, 80.967645f,
    80.018892f, 78.985289f, 78.008072f, 77.010186f,
    75.987642f, 75.038889f, 73.993502f, 73.002324f,
    72.016586f, 71.041000f, 69.955545f, 69.047766f,
    68.028486f, 67.008844f, 65.996635f, 64.976993f,
    63.963877f, 63.041957f, 62.031561f, 60.977471f,
    59.972333f, 58.960850f, 58.035666f, 57.026539f,
    56.017956f, 55.013181f, 54.022184f, 53.012151f,
    52.012633f, 50.977036f, 49.957575f, 48.986159f,
    48.027797f, 47.015588f, 45.993952f, 45.047193f,
    44.037341f, 42.962764f, 41.986634f, 41.030085f,
    40.014069f, 39.004217f, 37.988563f, 37.005000f,
    35.962514f, 35.021376f, 34.010436f, 32.978102f,
    32.002335f, 30.986863f, 29.991515f, 29.000700f,
    27.988672f, 27.030491f, 25.979665f, 25.020759f,
    24.014714f, 22.985101f, 21.995373f, 20.993499f,
    20.032779f, 18.995188f, 17.979716f, 17.000141f,
    15.996635f, 15.002012f, 13.988716f, 13.015668f,
    11.991312f, 10.991613f, 10.003517f, 9.493696f,
    8.991852f, 8.496717f, 7.998861f, 7.491397f,
    6.991910f, 6.501488f, 6.002364f, 5.500702f,
    5.001396f, 4.753194f, 4.506623f, 4.252619f,
    4.000972f, 3.747330f, 3.500760f, 3.252195f,
    3.002723f, 2.752526f, 2.499248f, 2.249595f,
    1.999942f, 1.748295f, 1.501180f, 1.248083f,
    0.909411f, 0.809876f, 0.709797f, 0.610262f,
    0.509821f, 0.409924f, 0.309301f, 0.209585f,
    0.110050f, 0.099897f, 0.095184f, 0.090288f,
    0.085031f, 0.079954f, 0.074878f, 0.069983f,
    0.065087f, 0.060011f, 0.054934f, 0.050039f,
    0.044963f, 0.039886f, 0.034991f, 0.029915f,
    0.025020f, 0.019943f, 0.015048f, 0.009972f,
    0.009065f, 0.007977f, 0.007071f, 0.005983f,
    0.005076f, 0.003989f, 0.003082f, 0.001994f,
    0.001088f, 0.000907f, 0.000725f, 0.000544f,
    0.000363f, 0.000181f };

const float VenusCroppedMX2X_FRRTable[] = {
    0.001000f, 0.003779f, 0.004972f, 0.005785f,
    0.009582f, 0.012728f, 0.014849f, 0.015736f,
    0.016546f, 0.017318f, 0.018012f, 0.018667f,
    0.019323f, 0.019902f, 0.021695f, 0.024588f,
    0.027480f, 0.030373f, 0.033059f, 0.035539f,
    0.037522f, 0.039671f, 0.041655f, 0.043638f,
    0.045622f, 0.049995f, 0.056359f, 0.062193f,
    0.068026f, 0.073860f, 0.080086f, 0.085045f,
    0.090500f, 0.095954f, 0.101409f, 0.106864f,
    0.112319f, 0.117501f, 0.123228f, 0.128956f,
    0.134683f, 0.141018f, 0.148250f, 0.156205f,
    0.164160f, 0.172115f, 0.180069f, 0.187330f,
    0.194330f, 0.201967f, 0.208967f, 0.217009f,
    0.227828f, 0.239630f, 0.251432f, 0.262251f,
    0.271719f, 0.281747f, 0.291004f, 0.300260f,
    0.310674f, 0.325716f, 0.340758f, 0.355800f,
    0.373156f, 0.391958f, 0.412207f, 0.433902f,
    0.449812f, 0.464999f, 0.480185f, 0.503327f,
    0.526468f, 0.549609f, 0.573792f, 0.596413f,
    0.621420f, 0.666763f, 0.712106f, 0.744171f,
    0.775412f, 0.809777f, 0.845704f, 0.886390f,
    0.942436f, 0.994215f, 1.047035f, 1.102401f,
    1.167920f, 1.246023f, 1.338617f, 1.365230f,
    1.393173f, 1.426020f, 1.460804f, 1.495516f,
    1.540208f, 1.590975f, 1.649696f, 1.702517f,
    1.751808f, 1.784784f, 1.819135f, 1.854064f,
    1.886462f, 1.909748f, 1.942797f, 1.987272f,
    2.026323f, 2.068340f, 2.118455f, 2.169395f,
    2.211310f, 2.272491f, 2.337504f, 2.436988f,
    2.623720f, 2.677466f, 2.724472f, 2.776396f,
    2.848568f, 2.967891f, 3.099026f, 3.283194f,
    3.593434f, 3.634365f, 3.650853f, 3.677562f,
    3.711503f, 3.743419f, 3.775528f, 3.805033f,
    3.843410f, 3.878122f, 3.930069f, 3.977437f,
    4.040498f, 4.095748f, 4.171898f, 4.224761f,
    4.301562f, 4.427394f, 4.609662f, 4.864478f,
    4.897888f, 4.987272f, 5.050815f, 5.152444f,
    5.226593f, 5.389066f, 5.655964f, 5.892874f,
    6.372963f, 6.415485f, 6.445859f, 6.703500f,
    7.884678f, 8.039533f
};

#ifdef __cplusplus
} //extern "C" {
#endif
