//!
//! Library of Constants and Definitions to make it
//! a bit easier to tweak the performance of the robots
//!

#![no_std]
#![feature(type_alias_impl_trait)]

extern crate alloc;

// Const Vals
pub const PI: f32 = 3.14159;
pub const MU: f32 = 4.0 * PI * 1e-7; // mu = 4 * pi * 10^-7
pub const BUTTON_DELAY: u32 = 200; // ms
pub const MOTOR_DELAY: u32 = 1;    // ms
pub const MAX_LENGTH: f32 = 20.0; // cm
pub const MAX_INDUCTANCE: f32 = 10000.0; // mH
pub const MAX_RADIUS: f32 = 5.0;  // cm
pub const WIRE_DIAMETER: f32 = 0.0511; // cm
pub const FULL_ROTATION: u32 = 200; // steps
pub const FEED_PER_FULL_ROTATION: f32 = 0.004 * FULL_ROTATION as f32; // cm
pub const FEED_PER_STEP: f32 = FEED_PER_FULL_ROTATION / (FULL_ROTATION as f32 * 2.0);
pub const FEED_STEP_PER_FULL_ROTATION: f32 = WIRE_DIAMETER / FEED_PER_STEP;
pub const COIL_STEP_PER_FEED_STEP: i32 = (FULL_ROTATION as f32 / FEED_STEP_PER_FULL_ROTATION).round() as i32;
pub const OFFSET: f32 = 0.5;  // cm
pub const PADDING: f32 = 0.1; // cm

pub mod peripherals;
pub use peripherals::*;
