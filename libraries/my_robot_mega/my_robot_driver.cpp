#include <my_motor_driver.h>



motor_driver::motor_driver()
{
    
}

void motor_driver::init()
{
    pinMode(A1, INPUT_PULLUP); //chan ngat encoder
    pinMode(B1, INPUT_PULLUP); //chan doc encoder
    pinMode(A2, INPUT_PULLUP); //chan ngat encoder
    pinMode(B2, INPUT_PULLUP); //chan doc encoder

    pinMode(EN_R, OUTPUT);   //chan pwm
    pinMode(INT1_R, OUTPUT); //chan DIR1
    pinMode(INT2_R, OUTPUT); //chan DIR2

    pinMode(EN_L, OUTPUT);   //chan pwm
    pinMode(INT1_L, OUTPUT); //chan DIR1
    pinMode(INT2_L, OUTPUT); //chan DIR2

    T = 0;
    speedR = 0.00, pre_speedR = 0.00;
    E1_R = 0, E1_1_R = 0, E1_2_R = 0;
    OutputR = 0, LastOutputR = 0;
    // KpR = 1000, KdR = 23.0, KiR = 10.0;

    speedL = 0.00, pre_speedL = 0.00;
    E1_L = 0, E1_1_L = 0, E1_2_L = 0;
    OutputL = 0, LastOutputL = 0;
    // KpL = 1200, KdL = 18.0, KiL = 8.0;
    KpL = 26, KdL = 15, KiL = 0.8;

    attachInterrupt(digitalPinToInterrupt(A1),cal_encoderL,CHANGE);
    attachInterrupt(digitalPinToInterrupt(A2),cal_encoderR,CHANGE);

}

void motor_driver::motor_Right(int Pulse_Width)
{
    if (Pulse_Width > 0)
    {
        analogWrite(EN_R, Pulse_Width);
        digitalWrite(INT1_R, LOW);
        digitalWrite(INT2_R, HIGH);
    }
    else if (Pulse_Width < 0)
    {
        analogWrite(EN_R, abs(Pulse_Width));
        digitalWrite(INT1_R, HIGH);
        digitalWrite(INT2_R, LOW);
    }
    else
    {
        analogWrite(EN_R, Pulse_Width);
        digitalWrite(INT1_R, LOW);
        digitalWrite(INT2_R, LOW);
    }
}

void motor_driver::motor_Left(int Pulse_Width)
{
    if (Pulse_Width > 0)
    {
        analogWrite(EN_L, Pulse_Width);
        digitalWrite(INT1_L, LOW);
        digitalWrite(INT2_L, HIGH);
    }
    else if (Pulse_Width < 0)
    {
        analogWrite(EN_L, abs(Pulse_Width));
        digitalWrite(INT1_L, HIGH);
        digitalWrite(INT2_L, LOW);
    }
    else
    {
        analogWrite(EN_L, Pulse_Width);
        digitalWrite(INT1_L, LOW);
        digitalWrite(INT2_L, LOW);
    }
}


// void motor_driver::read_EncoderL()
// {
//     if(digitalRead(B1)==HIGH)
//     {
//         pulsesL --;
//         pulseL_PID --;
//     }
//     else
//     {
//         pulsesL ++;
//         pulseL_PID ++;
//     }
// }

// void motor_driver::read_EncoderR()
// {
//     if(digitalRead(B2)==HIGH)
//     {
//         pulsesR ++;
//         pulseR_PID ++;
//     }
//     else
//     {
//         pulsesR --;
//         pulseR_PID --;
//     }
// }

void motor_driver::read_EncoderL()
{
    if ( digitalRead(B1) == 0 ) 
    {
        if ( digitalRead(A1) == 0 ) 
        {
            // A fell, B is low
            pulsesL ++; // Moving forward
	        pulseL_PID ++;
        } 
        else 
        {
            // A rose, B is high
            pulsesL --; // Moving reverse
	        pulseL_PID --;
        }
    } 
    else 
    {
        if ( digitalRead(A1) == 0 ) 
        {
            pulsesL --; // Moving reverse
            pulseL_PID --;
        } 
        else 
        {
            // A rose, B is low
            pulsesL ++; // Moving forward
	        pulseL_PID ++;
        }
    }
}

void motor_driver::read_EncoderR()
{
    if ( digitalRead(B2) == 0 ) 
    {
        if ( digitalRead(A2) == 0 ) 
        {
            // A fell, B is low
            pulsesR --; // Moving forward
	        pulseR_PID --;
        } 
        else
        {
            // A rose, B is high
            pulsesR ++; // Moving reverse
	        pulseR_PID ++;
        }
    } 
    else 
    {
        if ( digitalRead(A2) == 0 ) 
        {
            pulsesR ++; // Moving reverse
	        pulseR_PID ++;
        } 
        else 
        {
            // A rose, B is low
            pulsesR --; // Moving forward
	        pulseR_PID ++;
        }
    }
}

int32_t motor_driver::getLeftencoder()
{
    return pulsesL;
}

int32_t motor_driver::getRightencoder()
{
    return pulsesR;
}

void motor_driver::control_Motor(const float wheel_rad, const float wheel_sep, float* cmd_value, double time)
{
    T = time/1000;
    speedR = (pulseR_PID/616)*(1/T)*60;
    speedR = speedR * LPF_heso + pre_speedR * (1-LPF_heso);
    pre_speedR = speedR;
    pulseR_PID = 0;
    E1_R = setpointR - speedR;
    alphaR=2*T*KpR + KiR*T*T+ 2*KdR;
    betaR=T*T*KiR-4*KdR-2*T*KpR;
    gamaR=2*KdR;

    OutputR = (alphaR*E1_R + betaR*E1_1_R + gamaR*E1_2_R +2*T*LastOutputR/(2*T));
    LastOutputR = OutputR;
    E1_2_R = E1_1_R;
    E1_1_R = E1_R;
    
    speedL = (pulseL_PID/331)*(1/T)*60;
    speedL = speedL * LPF_heso + pre_speedL * (1-LPF_heso);
    pre_speedL = speedL;
    pulseL_PID = 0;
    E1_L = setpointL - speedL;

    alphaL=2*T*KpL + KiL*T*T+ 2*KdL;
    betaL=T*T*KiL-4*KdL-2*T*KpL;
    gamaL=2*KdL;

    OutputL = (alphaL*E1_L + betaL*E1_1_L + gamaL*E1_2_L +2*T*LastOutputL/(2*T));

    LastOutputL = OutputL;
    E1_2_L = E1_1_L;
    E1_1_L = E1_L;

    if(OutputR > 255)
    OutputR = 255;
    if(OutputR < -255)
    OutputR = -255;

    if(OutputL > 255)
    OutputL = 255;
    if(OutputL < -255)
    OutputL = -225;


    if (OutputR > 0)
    {
        analogWrite(EN_R, OutputR);
        digitalWrite(INT1_R, LOW);
        digitalWrite(INT2_R, HIGH);
    }
    else if (OutputR < 0)
    {
        analogWrite(EN_R, abs(OutputR));
        digitalWrite(INT1_R, HIGH);
        digitalWrite(INT2_R, LOW);
    }
    else
    {
        analogWrite(EN_R, OutputR);
        digitalWrite(INT1_R, LOW);
        digitalWrite(INT2_R, LOW);
    }

    if (OutputL > 0)
    {
        analogWrite(EN_L, OutputL);
        digitalWrite(INT1_L, LOW);
        digitalWrite(INT2_L, HIGH);
    }
    else if (OutputL < 0)
    {
        analogWrite(EN_L, abs(OutputL));
        digitalWrite(INT1_L, HIGH);
        digitalWrite(INT2_L, LOW);
    }
    else
    {
        analogWrite(EN_L, OutputL);
        digitalWrite(INT1_L, LOW);
        digitalWrite(INT2_L, LOW);
    }

}

void motor_driver::PID(double time)
{
    if(setpointL == 0)
    {
        analogWrite(EN_L, 0);
        digitalWrite(INT1_L, LOW);
        digitalWrite(INT2_L, LOW);
        pulseL_PID = 0;
        speedL = 0;
        E1_L = 0; E1_1_L = 0; E1_2_L = 0;
        alphaL = 0; betaL = 0; gamaL = 0;
        OutputL = 0;

        T = time/1000;
        speedR = (pulseR_PID/1000)*(1/T)*60;
        speedR = speedR * LPF_heso + pre_speedR * (1-LPF_heso);
        pre_speedR = speedR;
        pulseR_PID = 0;
        E1_R = setpointR - speedR;
        alphaR=2*T*KpR + KiR*T*T+ 2*KdR;
        betaR=T*T*KiR-4*KdR-2*T*KpR;
        gamaR=2*KdR;

        OutputR = (alphaR*E1_R + betaR*E1_1_R + gamaR*E1_2_R +2*T*LastOutputR/(2*T));
        LastOutputR = OutputR;
        E1_2_R = E1_1_R;
        E1_1_R = E1_R;

        if(OutputR > 255)
        OutputR = 255;
        if(OutputR < -255)
        OutputR = -255;

        if (OutputR > 0)
        {
            analogWrite(EN_R, OutputR);
            digitalWrite(INT1_R, LOW);
            digitalWrite(INT2_R, HIGH);
        }
        else if (OutputR < 0)
        {
            analogWrite(EN_R, abs(OutputR));
            digitalWrite(INT1_R, HIGH);
            digitalWrite(INT2_R, LOW);
        }
        else
        {
            analogWrite(EN_R, OutputR);
            digitalWrite(INT1_R, LOW);
            digitalWrite(INT2_R, LOW);
        }
    }
    else if (setpointR == 0)
    {
        analogWrite(EN_R, 0);
        digitalWrite(INT1_R, LOW);
        digitalWrite(INT2_R, LOW);
        pulseR_PID = 0;
        speedR = 0;
        E1_R = 0; E1_1_R = 0; E1_2_R = 0;
        alphaR = 0; betaR = 0; gamaR = 0;
        OutputR = 0;

        T = time/1000;
        speedL = (pulseL_PID/1000)*(1/T)*60;
        speedL = speedL * LPF_heso + pre_speedL * (1-LPF_heso);
        pre_speedL = speedL;
        pulseL_PID = 0;
        E1_L = setpointL - speedL;

        alphaL=2*T*KpL + KiL*T*T+ 2*KdL;
        betaL=T*T*KiL-4*KdL-2*T*KpL;
        gamaL=2*KdL;

        OutputL = (alphaL*E1_L + betaL*E1_1_L + gamaL*E1_2_L +2*T*LastOutputL/(2*T));
        LastOutputL = OutputL;
        E1_2_L = E1_1_L;
        E1_1_L = E1_L;

        if(OutputL > 255)
        OutputL = 255;
        if(OutputL < -255)
        OutputL = -225;

        if (OutputL > 0)
        {
            analogWrite(EN_L, OutputL);
            digitalWrite(INT1_L, LOW);
            digitalWrite(INT2_L, HIGH);
        }
        else if (OutputL < 0)
        {
            analogWrite(EN_L, abs(OutputL));
            digitalWrite(INT1_L, HIGH);
            digitalWrite(INT2_L, LOW);
        }
        else
        {
            analogWrite(EN_L, OutputL);
            digitalWrite(INT1_L, LOW);
            digitalWrite(INT2_L, LOW);
        }
    }
    else
    {
        T = time/1000;
        speedR = (pulseR_PID/1000)*(1/T)*60;
        speedR = speedR * LPF_heso + pre_speedR * (1-LPF_heso);
        pre_speedR = speedR;
        pulseR_PID = 0;
        E1_R = setpointR - speedR;
        alphaR=2*T*KpR + KiR*T*T+ 2*KdR;
        betaR=T*T*KiR-4*KdR-2*T*KpR;
        gamaR=2*KdR;

        OutputR = (alphaR*E1_R + betaR*E1_1_R + gamaR*E1_2_R +2*T*LastOutputR/(2*T));
        LastOutputR = OutputR;
        E1_2_R = E1_1_R;
        E1_1_R = E1_R;
        
        speedL = (pulseL_PID/1000)*(1/T)*60;
        speedL = speedL * LPF_heso + pre_speedL * (1-LPF_heso);
        pre_speedL = speedL;
        pulseL_PID = 0;
        E1_L = setpointL - speedL;

        alphaL=2*T*KpL + KiL*T*T+ 2*KdL;
        betaL=T*T*KiL-4*KdL-2*T*KpL;
        gamaL=2*KdL;

        OutputL = (alphaL*E1_L + betaL*E1_1_L + gamaL*E1_2_L +2*T*LastOutputL/(2*T));

        LastOutputL = OutputL;
        E1_2_L = E1_1_L;
        E1_1_L = E1_L;

        if(OutputR > 255)
        OutputR = 255;
        if(OutputR < -255)
        OutputR = -255;

        if(OutputL > 255)
        OutputL = 255;
        if(OutputL < -255)
        OutputL = -225;

        if (OutputR > 0)
        {
            analogWrite(EN_R, OutputR);
            digitalWrite(INT1_R, LOW);
            digitalWrite(INT2_R, HIGH);
        }
        else if (OutputR < 0)
        {
            analogWrite(EN_R, abs(OutputR));
            digitalWrite(INT1_R, HIGH);
            digitalWrite(INT2_R, LOW);
        }
        else
        {
            analogWrite(EN_R, OutputR);
            digitalWrite(INT1_R, LOW);
            digitalWrite(INT2_R, LOW);
        }

        if (OutputL > 0)
        {
            analogWrite(EN_L, OutputL);
            digitalWrite(INT1_L, LOW);
            digitalWrite(INT2_L, HIGH);
        }
        else if (OutputL < 0)
        {
            analogWrite(EN_L, abs(OutputL));
            digitalWrite(INT1_L, HIGH);
            digitalWrite(INT2_L, LOW);
        }
        else
        {
            analogWrite(EN_L, OutputL);
            digitalWrite(INT1_L, LOW);
            digitalWrite(INT2_L, LOW);
        }
    }
}

double motor_driver::getSpeedL()
{
    return speedL;
}

void motor_driver::setSetpointL(float spL)
{
    setpointL = spL;
}

void motor_driver::setSetpointR(float spR)
{
    setpointR = spR;
}

void motor_driver::setpulseL_PID(float pulse)
{
    pulseL_PID = pulse;
}

void motor_driver::setpulseR_PID(float pulse)
{
    pulseR_PID = pulse;
}