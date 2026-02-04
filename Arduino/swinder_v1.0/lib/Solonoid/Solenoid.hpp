#ifndef SOLENOID_HPP
#define SOLENOID_HPP

#include <Arduino.h>

#define MAX_LENGTH 2000 // 0.2m stored with 0.01cm precision. Divide by 10000
#define MAX_INDUCTANCE 4000000 // 40H stored with 0.01mH precision. Divide by 100000
#define MAX_RADIUS 500 // 0.005m stored with 0.01cm precision. Divide by 10000
#define MAX_GAUGE 12 // Semi arbitrary value representing number of gauge types

#define UT_SCALING_FACTOR 10 // 10^5
#define K 394784 // K = 4 * pi^2 * 10^-7 = ~394784 * 10^-11 for ~1.76 * 10^12 error

enum SolenoidError {
    NO_ERROR = 0,
    VALUE_ERROR = 1,
};

enum Preset {
    A,
    B,
    C,
    D,
    None,
    Debug,
};

enum WireGauge { // Diameter, divide by 1000000 to get m; SF 10^-6
    AWG18 = 0, // 1.020mm
    AWG19 = 1, // 0.910mm
    AWG20 = 2, // 0.810mm
    AWG21 = 3, // 0.720mm
    AWG22 = 4, // 0.643mm
    AWG23 = 5, // 0.574mm
    AWG24 = 6, // 0.511mm
    AWG25 = 7, // 0.450mm
    AWG26 = 8, // 0.404mm
    AWG27 = 9, // 0.361mm
    AWG28 = 10, // 0.320mm
    AWG29 = 11, // 0.290mm
    AWG30 = 12, // 0.254mm
};

class Solenoid {
public:
    /**
     * @brief Create a new instance of the solenoid driver.
     */
    Solenoid();

    /**
     * @brief Initializes a solenoid from the given preset to garuntee error free.
     * 
     * @param preset the Preset to default the values to
     */
    void begin(Preset preset);

    /**
     * @brief Getter for solenoid length
     * 
     * @returns length of solenoid
     */
    uint32_t getLength();

    /**
     * @brief Getter for solenoid radius
     * 
     * @returns radius of solenoid
     */
    uint32_t getRadius();

    /**
     * @brief Getter for solenoid inductance
     * 
     * @returns inductance of solenoid
     */
    uint32_t getInductance();

    /**
     * @brief Getter for solenoid wire gauge
     * 
     * @returns gauge of solenoid
     */
    WireGauge getGauge();

    /**
     * @brief Getter for number of turns required for solenoid
     * 
     * Only updates turns count on call
     * 
     * @returns number of turns for solenoid
     */
    uint32_t getTurns();

    /**
     * @brief Setter for solenoid length
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setLength(uint32_t length);

    /**
     * @brief Setter for solenoid radius
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setRadius(uint32_t radius);

    /**
     * @brief Setter for solenoid inductance
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setInductance(uint32_t inductance);

    /**
     * @brief Setter for solenoid wire gauge
     * 
     * @returns VAL_ERROR if input value is outside of expected range
     */
    SolenoidError setGauge(WireGauge gauge);

    /**
     * @brief Setter for solenoid wire gauge using gauge/index value
     * 
     * @returns VAL_ERROR if input value is outside expected range
     */
    SolenoidError setGauge(uint32_t gauge);

    /**
     * @brief Updates all values of solenoid to predefined presets
     * 
     * @param preset selected preset
     */
    void setPreset(Preset preset);

    /**
     * @brief Provides a string format for gauge
     * 
     * @returns String representation of the gauge
     */
    String gaugeString();

    /**
     * @brief Returns real value of gauge diameter
     * 
     * @returns diameter with 0.001mm precision
     */
    uint32_t gaugeDiameter();

    /**
     * @brief Returns the number of turns that can fit in one pass across the solenoid
     * 
     * @returns number of turns per pass
     */
    uint32_t turnsPerPass();

    /**
     * @brief Overrides current turn count
     * 
     * @param turns number of turns, input -1 to reset to calculated value
     */
    void turnsOverride(int32_t turns);

    /**
     * @brief Getter for the status of override
     * 
     * @returns status of override
     */
    bool getOverride();

private:
    /**
     * @brief Calculates the number of turns required for solenoid
     * 
     * Equation: sqrt((inductance * length) / (R^2 * K))
     */
    void updateTurns();

    uint64_t _length = 0;
    uint64_t _radius = 0;
    uint64_t _inductance = 0;
    WireGauge _gauge = WireGauge::AWG24;
    uint64_t _numTurns = 0;
    bool _override = false;
};

#endif