use teensy4_pins::t41::*;
use teensy4_bsp::board::{self, PERCLK_FREQUENCY};
use teensy4_bsp::hal::{
    gpio::{Input, Output, Port},
    gpt::{Gpt1, Gpt2},
    pit::Pit2,
    timer::Blocking,
};

pub const GPT_FREQUENCY: u32 = 1_000;

// Pin Assignments
pub const COIL_MOTOR_STEP_PIN: u32 = 36;
pub const COIL_MOTOR_DIR_PIN: u32 = 37;
pub const COIL_MOTOR_FAULT_PIN: u32 = 29;
pub const COIL_MOTOR_SLEEP_PIN: u32 = 35;

pub const FEED_MOTOR_STEP_PIN: u32 = 38;
pub const FEED_MOTOR_DIR_PIN: u32 = 39;
pub const FEED_MOTOR_FAULT_PIN: u32 = 30;
pub const FEED_MOTOR_SLEEP_PIN: u32 = 40;

pub const RBUTTON_PIN: u32 = 23;
pub const RA_PIN: u32 = 22;
pub const RB_PIN: u32 = 21;

pub const LIMIT_SWITCH_START_PIN: u32 = 7;
pub const LIMIT_SWITCH_END_PIN: u32 = 8;

// import libraries
use teensy4_bsp::hal::{i2c::I2C, gpio::*, timer::Timer};
use lcd1602_driver::{Lcd1602, I2CAddress};
use rotary_encoder_embedded::RotaryEncoder;
use stepper_driver::StepperDriver;
use teensy4_pins::t41::*;

/// The general-purpose delay shared by different peripherals
pub type Delay2 = Blocking<Gpt2, GPT_FREQUENCY>;
/// The PIT-defined delay for initializing the IMU.
pub type PitDelay = Blocking<Pit2, PERCLK_FREQUENCY>;
/// The first GPIO port
pub type Gpio1 = Port<1>;
/// The second GPIO port
pub type Gpio2 = Port<2>;
/// And so on
pub type Gpio3 = Port<3>;
pub type Gpio4 = Port<4>;
