#include "Solenoid.hpp"

Solenoid::Solenoid() {}

// PUBLIC

void Solenoid::begin(Preset preset) {
    this->setPreset(preset);
}

SolenoidError Solenoid::setLength(uint32_t length) {
    if (length > MAX_LENGTH) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_length = length;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setRadius(uint32_t radius) {
    if (radius > MAX_LENGTH) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_radius = radius;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setInductance(uint32_t inductance) {
    if (inductance > MAX_INDUCTANCE) {
        return SolenoidError::VALUE_ERROR;
    }
    this->_inductance = inductance;
    return SolenoidError::NO_ERROR;
}

SolenoidError Solenoid::setGauge(WireGauge gauge) {
    this->_gauge = gauge;
    return SolenoidError::NO_ERROR;
}

uint32_t Solenoid::getLength() {
    return _length;
}

uint32_t Solenoid::getRadius() {
    return _radius;
}

uint32_t Solenoid::getInductance() {
    return _inductance;
}

WireGauge Solenoid::getGauge() {
    return _gauge;
}

uint32_t Solenoid::getTurns() {
    if (!_override) {
        this->updateTurns();
    }
    return _numTurns;
}

void Solenoid::setPreset(Preset preset) {
    switch (preset) {
        case Preset::A:
            this->setLength((uint32_t) 425); // 5cm
            this->setRadius((uint32_t) 65); // 0.65cm
            this->setInductance((uint32_t) 400); // 4mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::B:
            this->setLength((uint32_t) 500); // 5cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 8000); // 80mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::C:
            this->setLength((uint32_t) 300); // 3cm
            this->setRadius((uint32_t) 50); // 0.5cm
            this->setInductance((uint32_t) 4000); // 40mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::D:
            this->setLength((uint32_t) 100); // 1cm
            this->setRadius((uint32_t) 100); // 1cm
            this->setInductance((uint32_t) 10); // 0.1mH
            this->setGauge(WireGauge::AWG24);
        break;
        case Preset::None:
            this->setLength((uint32_t) 0); // 0cm
            this->setRadius((uint32_t) 0); // 0cm
            this->setInductance((uint32_t) 0); // 0mH
            this->setGauge(WireGauge::AWG24);
        break;
        default: // debug case
            this->setLength((uint32_t) 1234); // 0.1234m
            this->setRadius((uint32_t) 123); // 0.0123m
            this->setInductance((uint32_t) 1234567); // 12.34567H
            this->setGauge(WireGauge::AWG24);
    }
}

uint32_t Solenoid::turnsPerPass() {
    if (_length == 0) {
        return 0;
    }
    // (_length / 10000) / (_gauge / 1000000)
    return (_length / _gauge) * 100;
}

void Solenoid::turnsOverride(int32_t turns) {
    this->updateTurns();
    if (turns < 0 || uint(turns) == _numTurns) {
        _override = false;
        return;
    }
    _override = true;
    this->_numTurns = turns;
}

bool Solenoid::getOverride() {
    return _override;
}


String Solenoid::gaugeString() {
    switch(_gauge) {
        case WireGauge::AWG18: return "AWG18";
        case WireGauge::AWG19: return "AWG19";
        case WireGauge::AWG20: return "AWG20";
        case WireGauge::AWG21: return "AWG21";
        case WireGauge::AWG22: return "AWG22";
        case WireGauge::AWG23: return "AWG23";
        case WireGauge::AWG24: return "AWG24";
        case WireGauge::AWG25: return "AWG25";
        case WireGauge::AWG26: return "AWG26";
        case WireGauge::AWG27: return "AWG27";
        case WireGauge::AWG28: return "AWG28";
        case WireGauge::AWG29: return "AWG29";
        case WireGauge::AWG30: return "AWG30";
        default: return "Error";
    }
}

uint32_t Solenoid::gaugeDiameter() {
    
    switch (_gauge) { // Diameter, divide by 1000000 to get m; SF 10^-6
        case WireGauge::AWG18: return 1020; // 1.020mm
        case WireGauge::AWG19: return 910; // 0.910mm
        case WireGauge::AWG20: return 810; // 0.810mm
        case WireGauge::AWG21: return 720; // 0.720mm
        case WireGauge::AWG22: return 643; // 0.643mm
        case WireGauge::AWG23: return 574; // 0.574mm
        case WireGauge::AWG24: return 511; // 0.511mm
        case WireGauge::AWG25: return 450; // 0.450mm
        case WireGauge::AWG26: return 404; // 0.404mm
        case WireGauge::AWG27: return 361; // 0.361mm
        case WireGauge::AWG28: return 320; // 0.320mm
        case WireGauge::AWG29: return 290; // 0.290mm
        case WireGauge::AWG30: return 254; // 0.254mm
        default: return 0;
    }
}

// PRIVATE

void Solenoid::updateTurns() {
    if (_radius == 0 || _length == 0 || _inductance == 0) {
        this->_numTurns = 0;
        return;
    }
    this->_numTurns = round(sqrt(((_inductance * _length * 100000000) / (_radius * _radius * K))) * UT_SCALING_FACTOR);
}