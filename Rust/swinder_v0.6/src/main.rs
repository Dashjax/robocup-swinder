#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use embedded_alloc::Heap;

#[global_allocator]
static HEAP: Heap = Heap::empty();

use teensy4_panic as _;

#[rtic::app(device = teensy4_bsp, peripherals = true, dispatchers = [GPT2])]
mod app {
    use swinder::{Gpio1, Gpio2, Gpio3, Gpio4, PitDelay, Delay2, GPT_FREQUENCY};
    

    #[local]
    struct Local {

    }
    
    #[shared]
    struct Shared {
        length: f32,       // In cm
        inductance: f32,   // In mH
        radius: f32,       // In cm
        num_turns: i32,    // Total turns

        lcd: Lcd1602<I2C<I2C1>>,
        encoder: RotaryEncoder,
        stepper_motor: StepperDriver,
        }
    
    #[init]
    fn init(ctx: init::Context) -> (Shared, Local) {
        let shared = Shared {
            length: 5.0,
            inductance: 40.0,
            radius: 0.5,
            num_turns: 0,
            lcd: {
                let i2c = I2C::new(ctx.device.I2C1, &mut ctx.device.PCC);
                Lcd1602::new(i2c, I2CAddress::default())
            },
            encoder: RotaryEncoder::new(ctx.device.GPIO, RA_PIN, RB_PIN),
            stepper_motor: StepperDriver::new(
                ctx.device.GPIO,
                COIL_MOTOR_STEP_PIN,
                COIL_MOTOR_DIR_PIN,
                COIL_MOTOR_SLEEP_PIN,
            ),
        };

        (Shared {}, Local {})
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        loop {
            cortex_m::asm::wfi();
        }
    }
    
    #[task(priority = 1)]
    async fn temp(_ctx: temp::Context) {

        ctx.shared.lcd.lock(|lcd| {

        });
    
        ctx.shared.encoder.lock(|encoder| {
            //let position = encoder.get_position();
        });
    
        ctx.shared.stepper_motor.lock(|motor| {
            
        });


        loop {
            
        }
    }
}

