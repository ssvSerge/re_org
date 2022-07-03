#include "far_lut.h"


//====================================================================================================================
// --- Common --- //

// Search an ordered table of increasing values
// and return the indices which bracket the input value of T
void locate1(unsigned int T, const unsigned int* Table, int nTableSize, int& i1, int& i2)
{
    i1 = 0;
    i2 = nTableSize - 1;
    while (i2 - i1 > 1)
    {
        //Bracket the given value
        int im = (i2 + i1) / 2;
        if (T > Table[im])
            i1 = im;
        else
            i2 = im;
    }
}


// Search an ordered table of decreasing values
// and return the indices which bracket the input value of T
void locate2(float T, const float* Table, int nTableSize, int& i1, int& i2)
{
    i1 = 0;
    i2 = nTableSize - 1;
    while (i2 - i1 > 1)
    {
        //Bracket the given value
        int im = (i2 + i1) / 2;
        if (T < Table[im])
            i1 = im;
        else
            i2 = im;
    }
}



//====================================================================================================================
// --- V-Series --- //

float GetFARForFullSized(unsigned int Threshold)
{

  if(Threshold <= FullSized_ThresholdTable[0])
    return FullSized_FARTable[0];

  if(Threshold >= FullSized_ThresholdTable[FullSized_Table_Size - 1])
  {
    return FullSized_FARTable[FullSized_Table_Size - 1];
  }

  //Find indices below and above
  int i1 = 0;
  int i2 = 0;
  locate1(Threshold, FullSized_ThresholdTable, FullSized_Table_Size, i1, i2);

  //Perform a linear interpolation between the
  //value below and above
  float x1 = FullSized_ThresholdTable[i1];
  float y1 = FullSized_FARTable[i1];
  float x2 = FullSized_ThresholdTable[i2];
  float y2 = FullSized_FARTable[i2];
  if((x1 == x2) || (y1 == y2))
    return y1;

  float m = (y2 - y1) / (x2 - x1);
  float b = y1 - m * x1;
  return m * Threshold + b;
}

float GetFRRForFullSized(unsigned int Threshold)
{

  if(Threshold <= FullSized_ThresholdTable[0])
    return FullSized_FRRTable[0];

  if(Threshold >= FullSized_ThresholdTable[FullSized_Table_Size - 1])
  {
    return FullSized_FRRTable[FullSized_Table_Size - 1];
  }

  //Find indices below and above
  int i1 = 0;
  int i2 = 0;
  locate1(Threshold, FullSized_ThresholdTable, FullSized_Table_Size, i1, i2);

  //Perform a linear interpolation between the
  //value below and above
  float x1 = FullSized_ThresholdTable[i1];
  float y1 = FullSized_FRRTable[i1];
  float x2 = FullSized_ThresholdTable[i2];
  float y2 = FullSized_FRRTable[i2];
  if((x1 == x2) || (y1 == y2))
    return y1;

  float m = (y2 - y1) / (x2 - x1);
  float b = y1 - m * x1;
  return m * Threshold + b;
}

//Cropped
float GetFARForCropped(unsigned int Threshold)
{

  if(Threshold <= Cropped_ThresholdTable[0])
    return Cropped_FARTable[0];

  if(Threshold >= Cropped_ThresholdTable[Cropped_Table_Size - 1])
  {
    return Cropped_FARTable[Cropped_Table_Size - 1];
  }

  //Find indices below and above
  int i1 = 0;
  int i2 = 0;
  locate1(Threshold, Cropped_ThresholdTable, Cropped_Table_Size, i1, i2);

  //Perform a linear interpolation between the
  //value below and above
  float x1 = Cropped_ThresholdTable[i1];
  float y1 = Cropped_FARTable[i1];
  float x2 = Cropped_ThresholdTable[i2];
  float y2 = Cropped_FARTable[i2];
  if((x1 == x2) || (y1 == y2))
    return y1;

  float m = (y2 - y1) / (x2 - x1);
  float b = y1 - m * x1;
  return m * Threshold + b;
}

float GetFRRForCropped(unsigned int Threshold)
{

  if(Threshold <= Cropped_ThresholdTable[0])
    return Cropped_FRRTable[0];

  if(Threshold >= Cropped_ThresholdTable[Cropped_Table_Size - 1])
  {
    return Cropped_FRRTable[Cropped_Table_Size - 1];
  }

  //Find indices below and above
  int i1 = 0;
  int i2 = 0;
  locate1(Threshold, Cropped_ThresholdTable, Cropped_Table_Size, i1, i2);

  //Perform a linear interpolation between the
  //value below and above
  float x1 = Cropped_ThresholdTable[i1];
  float y1 = Cropped_FRRTable[i1];
  float x2 = Cropped_ThresholdTable[i2];
  float y2 = Cropped_FRRTable[i2];
  if((x1 == x2) || (y1 == y2))
    return y1;

  float m = (y2 - y1) / (x2 - x1);
  float b = y1 - m * x1;
  return m * Threshold + b;
}


float GetThresholdFromFARForFullSized(float FAR)
{

  if(FAR >= FullSized_FARTable[0])
    return FullSized_ThresholdTable[0];

  if(FAR <= FullSized_FARTable[FullSized_Table_Size - 1])
  {
    return FullSized_ThresholdTable[FullSized_Table_Size - 1];
  }

  //Find indices below and above
  int i1 = 0;
  int i2 = 0;
  locate2(FAR, FullSized_FARTable, FullSized_Table_Size, i1, i2);

  //Perform a linear interpolation between the
  //value below and above
  float x1 = FullSized_FARTable[i1];
  float y1 = FullSized_ThresholdTable[i1];
  float x2 = FullSized_FARTable[i2];
  float y2 = FullSized_ThresholdTable[i2];
  if((x1 == x2) || (y1 == y2))
    return y1;

  float m = (y2 - y1) / (x2 - x1);
  float b = y1 - m * x1;
  return m * FAR + b;
}

//====================================================================================================================
// --- M-Series --- //


float GetFARForMX2X(unsigned int Threshold)
{

    if (Threshold <= MX2X_ThresholdTable[0])
        return MX2X_FARTable[0];

    if (Threshold >= MX2X_ThresholdTable[MX2X_Table_Size - 1])
    {
        return MX2X_FARTable[MX2X_Table_Size - 1];
    }

    //Find indices below and above
    int i1 = 0;
    int i2 = 0;
    locate1(Threshold, MX2X_ThresholdTable, MX2X_Table_Size, i1, i2);

    //Perform a linear interpolation between the
    //value below and above
    float x1 = MX2X_ThresholdTable[i1];
    float y1 = MX2X_FARTable[i1];
    float x2 = MX2X_ThresholdTable[i2];
    float y2 = MX2X_FARTable[i2];
    if ((x1 == x2) || (y1 == y2))
        return y1;

    float m = (y2 - y1) / (x2 - x1);
    float b = y1 - m * x1;
    return m * Threshold + b;
}

float GetFRRForMX2X(unsigned int Threshold)
{
    if (Threshold <= MX2X_ThresholdTable[0])
        return MX2X_FRRTable[0];

    if (Threshold >= MX2X_ThresholdTable[MX2X_Table_Size - 1])
    {
        return MX2X_FRRTable[MX2X_Table_Size - 1];
    }

    //Find indices below and above
    int i1 = 0;
    int i2 = 0;
    locate1(Threshold, MX2X_ThresholdTable, MX2X_Table_Size, i1, i2);

    //Perform a linear interpolation between the
    //value below and above
    float x1 = MX2X_ThresholdTable[i1];
    float y1 = MX2X_FRRTable[i1];
    float x2 = MX2X_ThresholdTable[i2];
    float y2 = MX2X_FRRTable[i2];
    if ((x1 == x2) || (y1 == y2))
        return y1;

    float m = (y2 - y1) / (x2 - x1);
    float b = y1 - m * x1;
    return m * Threshold + b;


}

float GetFARForVenusCroppedMX2X(unsigned int Threshold)
{

    if (Threshold <= VenusCroppedMX2X_ThresholdTable[0])
        return VenusCroppedMX2X_FARTable[0];

    if (Threshold >= VenusCroppedMX2X_ThresholdTable[VenusCroppedMX2X_Table_Size - 1])
    {
        return VenusCroppedMX2X_FARTable[VenusCroppedMX2X_Table_Size - 1];
    }

    //Find indices below and above
    int i1 = 0;
    int i2 = 0;
    locate1(Threshold, VenusCroppedMX2X_ThresholdTable, VenusCroppedMX2X_Table_Size, i1, i2);

    //Perform a linear interpolation between the
    //value below and above
    float x1 = VenusCroppedMX2X_ThresholdTable[i1];
    float y1 = VenusCroppedMX2X_FARTable[i1];
    float x2 = VenusCroppedMX2X_ThresholdTable[i2];
    float y2 = VenusCroppedMX2X_FARTable[i2];
    if ((x1 == x2) || (y1 == y2))
        return y1;

    float m = (y2 - y1) / (x2 - x1);
    float b = y1 - m * x1;
    return m * Threshold + b;
}

float GetFRRForVenusCroppedMX2X(unsigned int Threshold)
{

    if (Threshold <= VenusCroppedMX2X_ThresholdTable[0])
        return VenusCroppedMX2X_FRRTable[0];

    if (Threshold >= VenusCroppedMX2X_ThresholdTable[VenusCroppedMX2X_Table_Size - 1])
    {
        return VenusCroppedMX2X_FRRTable[VenusCroppedMX2X_Table_Size - 1];
    }

    //Find indices below and above
    int i1 = 0;
    int i2 = 0;
    locate1(Threshold, VenusCroppedMX2X_ThresholdTable, VenusCroppedMX2X_Table_Size, i1, i2);

    //Perform a linear interpolation between the
    //value below and above
    float x1 = VenusCroppedMX2X_ThresholdTable[i1];
    float y1 = VenusCroppedMX2X_FRRTable[i1];
    float x2 = VenusCroppedMX2X_ThresholdTable[i2];
    float y2 = VenusCroppedMX2X_FRRTable[i2];
    if ((x1 == x2) || (y1 == y2))
        return y1;

    float m = (y2 - y1) / (x2 - x1);
    float b = y1 - m * x1;
    return m * Threshold + b;
}

float GetThresholdFromFARForMX2X(float FAR)
{

    if (FAR >= MX2X_FARTable[0])
        return MX2X_ThresholdTable[0];

    if (FAR <= MX2X_FARTable[MX2X_Table_Size - 1])
    {
        return MX2X_ThresholdTable[MX2X_Table_Size - 1];
    }

    //Find indices below and above
    int i1 = 0;
    int i2 = 0;
    locate2(FAR, MX2X_FARTable, MX2X_Table_Size, i1, i2);

    //Perform a linear interpolation between the
    //value below and above
    float x1 = MX2X_FARTable[i1];
    float y1 = MX2X_ThresholdTable[i1];
    float x2 = MX2X_FARTable[i2];
    float y2 = MX2X_ThresholdTable[i2];
    if ((x1 == x2) || (y1 == y2))
        return y1;

    float m = (y2 - y1) / (x2 - x1);
    float b = y1 - m * x1;
    return m * FAR + b;
}

