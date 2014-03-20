/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj.templates;


import edu.wpi.first.wpilibj.Compressor;
import edu.wpi.first.wpilibj.IterativeRobot;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.RobotDrive;
import edu.wpi.first.wpilibj.Solenoid;
import edu.wpi.first.wpilibj.Victor;

/**
 * The VM is configured to automatically run this class, and to call the
 * functions corresponding to each mode, as described in the IterativeRobot
 * documentation. If you change the name of this class or the package after
 * creating this project, you must also update the manifest file in the resource
 * directory.
 */
public class RobotTemplate extends IterativeRobot {
        /** Right Joystick Buttons **/
    private static final int shiftButton = 3; //Right Joystick button 3 is the shifter
   
    
    /** IO Definitions **/
    /* Motors */
    private static final int MotorDriveRL = 1;
    private static final int MotorDriveRR = 2;
    private static final int MotorDriveFL = 3;
    private static final int MotorDriveFR = 4;
    private static final int MotorDriveML = 5;
    private static final int MotorDriveMR = 6;    
 
    /* Digital IO */
    private static final int DIOPressureSwitch = 1;
    
    /* Solenoids - Module 1 */
    private static final int SolenoidDriveShifter = 1;

    /* Relays */
    private static final int RelayCompressor = 1;

    
    Victor rearLeftMotor = new Victor(MotorDriveRL);
    Victor rearRightMotor = new Victor(MotorDriveRR);

    Victor frontLeftMotor = new Victor(MotorDriveFL);
    Victor frontRightMotor = new Victor(MotorDriveFR);
        
    Victor midLeftMotor = new Victor(MotorDriveML);
    Victor midRightMotor = new Victor(MotorDriveMR);                
    
    //RobotDrive6 drive = new RobotDrive6(frontLeftMotor,midLeftMotor, rearLeftMotor,frontRightMotor,midRightMotor,rearRightMotor);
    RobotDrive drive = new RobotDrive(frontLeftMotor,rearLeftMotor,frontRightMotor,rearRightMotor);
    //RobotDrive drive2 = new RobotDrive(midLeftMotor, midRightMotor);
    //RobotDrive6 drive = new RobotDrive6(frontLeftMotor, rearLeftMotor, frontRightMotor, rearRightMotor); //For 4 motor drivetrain
    
    /* Instansiate Joysticks */
    Joystick leftStick = new Joystick(1);
    Joystick rightStick = new Joystick(2);
    Joystick operatorStick = new Joystick(3);
    
        /* Pnumatics */
    Compressor compressor = new Compressor(DIOPressureSwitch,RelayCompressor);  
    Solenoid shifter = new Solenoid(SolenoidDriveShifter);

    boolean lastShiftButton = false;

    
    
    /**
     * This function is run when the robot is first started up and should be
     * used for any initialization code.
     */
    public void robotInit() {
        /* Start Compressor - logic handled by Compressor class */
        compressor.start();
        getWatchdog().setEnabled(false);
    }

    /**
     * This function is called periodically during autonomous
     */
    public void autonomousPeriodic() {

    }

    /**
     * This function is called periodically during operator control
     */
    public void teleopPeriodic() {
      
        drive.tankDrive(rightStick, leftStick); // drive with the joysticks 
            
        if(!lastShiftButton && rightStick.getRawButton(1))
        {
            shifter.set(!shifter.get());
        }
        lastShiftButton = rightStick.getRawButton(1);        
    }
    
    /**
     * This function is called periodically during test mode
     */
    public void testPeriodic() {
    
    }
    
}
