//
//  Insensitive.h
//  Insensitive
//
//  Created by Klaus Boldt on 16.11.11.
//  Copyright (c) 2009-2023 Klaus Boldt. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <string.h>
#include <cblas.h>
#include <fftw3.h>
#include <cairo.h>
#include <pango/pango.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifndef Insensitive_h
#define Insensitive_h

#define initial_chemical_shift 0.0
#define initial_scalar_coupling_constant 0.0
#define initial_distance 0.3
#define M_LN16 2.772588722239781144907055931980721653 /* loge(16) */

static const unsigned int spinTypeI = 0;
static const unsigned int spinTypeS = 1;
static const float gyro_1H = 267.522e6;
static const float gyro_13C = 67.283e6;
static const float gyro_15N = -27.128e6;
static const float gyro_19F = 251.815e6;
static const float gyro_29Si = -53.190e6;
static const float gyro_31P = 108.394e6;
static const float gyro_57Fe = 8.661e6;
static const float gyro_77Se = 51.2e6;
static const float gyro_113Cd = -59.55e6;
static const float gyro_119Sn = -100.138e6;
static const float gyro_129Xe = -74.41e6;
static const float gyro_183W = 11.2e6;
static const float gyro_195Pt = 57.68e6;
static const float h_bar = 1.055e-34;
static const float cos45 = M_SQRT1_2;
static const float sin45 = -M_SQRT1_2;
static const unsigned int maxNumberOfSpins = 4;
static const unsigned int gradientSlices = 201;
static const float spectrometer_frequency = 500.132249206e6; //500e6;
static const unsigned int pulseShapeResolution = 256;
static const int pulsePowerSpectrumResolution = 512;
static const int pulsePowerSpectrumCenter = 256; //pulsePowerSpectrumResolution / 2;
static const int pulsePowerSpectrumQuarter = 128; //pulsePowerSpectrumResolution / 4;
static const float minimumStepSize = 0.001;
static const unsigned int maxDanteCycles = 8;
static const float LB_Traficante = 0.03;
static const float Jan_1_2001 = 978303600.0;
static const char *insensitive_version = "0.9.34\0";
static const int energy_level_str_len = 156;

enum CouplingMode {
    WeakCouplingMode,
    StrongCouplingMode,
    SpinLockMode
};

enum VectorDisplayType {
    VectorDisplayTypeCoherences,
    VectorDisplayTypeMoments,
    VectorDisplayTypeFID
};

enum VectorDiagramType {
    VectorDiagram3D,
    VectorDiagramXYplane,
    VectorDiagramGrapefruit
};

enum OperatorBasis {
    CartesianOperatorBasis,
    SphericalOperatorBasis
};

enum MatrixDisplayType {
    MatrixDisplayTypeHidden,
    MatrixDisplayTypeTiny,
    MatrixDisplayTypeSmall,
    MatrixDisplayTypeLarge,
    MatrixDisplayTypeGraphical
};

enum WindowFunctionType {
    WFNone = 0,
    WFExp = 1,
    WFCosine = 2,
    WFTriangle = 3,
    WFHann = 4,
    WFSineBell = 5,
    WFWeightedHann = 6,
    WFTraficante = 7,
    WFTraficanteSN = 8,
    WFLorentzGaussTransformation = 9,
    WFGaussPseudoEchoTransformation = 10
};

enum SpinEditorConstant {
    ScalarConstant,
    DipolarConstant,
    DistanceConstant
};

enum PurePhaseDetectionMethod {
    None = 0,
    States = 1,
    TPPI = 2,
    StatesTPPI = 3,
    EchoAntiecho = 4,
    QSEQ = 5
};

enum SpectrumDimension {
    F1,
    F2
};

enum PulseEnvelope {
    Rectangle,
    Gaussian,
    Sinc,
    EBURP_1,
    EBURP_2,
    IBURP_1,
    IBURP_2,
    UBURP,
    REBURP,
    DANTE,
    HypSec
};

enum QuickPulse {
    pulse90x,
    pulse90y,
    pulse90minusx,
    pulse90minusy,
    pulse180x,
    pulse180y,
    pulse180minusx,
    pulse180minusy,
    NoPulseApplied
};

enum DataSetType {
    FID,
    FFT1D,
    FFT2D,
    AbsValue
};

enum PlotMode {
    Contours,
    Stacked,
    Raster
};

enum OpenFileState {
    NoFile,
    FileOpened,
    FileOpenedAndChanged
};

enum ExportFormat {
    CSV,
    DAT,
    JDX,
    TXT,
    PNG
};


enum ExcitationProfile {
    Mx_My,
    Mxy_Phase,
    Mz
};

enum SettingsPickerViewContents {
    PulseShapeNames,
    DelayTime,
    VectorDisplay,
    OperatorBasis,
    DataPoints
};

enum Symmetrization {
    SYMA = -2,
    SYM = -1,
    SYMJ = 0,
    SYM_P = 1,
    SYMA_P = 2
};

typedef struct {
    unsigned int size;
    float *x, *y, *z;
    unsigned int *spinType;
    char *selected;
} VectorCoordinates;

enum PresentationObjectType {
    NoStyle,
    VectorModelISpins,
    VectorModelSSpins,
    DensityMatrix,
    ProductOperators,
    EnergyLevels,
    SpinNetwork,
    PulseProgram,
    CoherencePathway,
    PhaseCyclingTable,
    PulseShape,
    PulsePowerSpectrum,
    Spectrum,
    TextField,
    Image,
    Controls,
    SingleSpins
};

typedef struct {
    float A;
    int x, y;
    float wx, wy;
} FittedSpectrum;

typedef struct {
    double A_alpha, A_beta;
    double phase_alpha, phase_beta;
} SingleSpinVector;

typedef struct {
    unsigned int index;
    float x1, y1, x2, y2;
} ContourLine;

// Standard values
#define standardFlipAngle 90
#define standardPulseDuration 0.1
#define standardPulseStrength 2.5
#define standardPhase 0
#define standardPulseArray 3
#define standardPulseLength 1
#define standardPulseFrequency 0
#define standardRelaxationWithEvolution FALSE
#define standardT1 10
#define standardT2 10
#define standardCorrelationTime 0.2e-9
#define standardDelay 1.0
#define standardDecoupling FALSE
#define standardGradientStrength 32000
#define standardGradientDuration 1.0
#define standardDataPoints 512
#define standardDwellTime 0.1
#define standardNoiseLevel 0.0
#define standardDetectISpins TRUE
#define standardDetectSSpins FALSE
#define standardVectorDisplayType VectorDisplayTypeCoherences
#define standardOperatorBasis CartesianOperatorBasis
#define standardColor1stOrderCoherences TRUE
#define standardMatrixDisplayType MatrixDisplayTypeSmall
#define standardAllowShiftAndCouplingButtons FALSE
#define standardPlaySound FALSE
#define standardVectorDiagramType 0
#define standardSignalToNoiseThreshold 0.0
#define standardMaxCoherenceCalculations 6.0

#endif
