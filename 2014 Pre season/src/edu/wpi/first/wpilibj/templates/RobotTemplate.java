/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj.templates;



import edu.wpi.first.wpilibj.IterativeRobot;
import edu.wpi.first.wpilibj.Victor;
import edu.wpi.first.wpilibj.RobotDrive;
import edu.wpi.first.wpilibj.Joystick;
import edu.wpi.first.wpilibj.RobotDrive.MotorType;

/**
 * The VM is configured to automatically run this class, and to call the
 * functions corresponding to each mode, as described in the IterativeRobot
 * documentation. If you change the name of this class or the package after
 * creating this project, you must also update the manifest file in the resource
 * directory.
 */
public class RobotTemplate extends IterativeRobot {
    private Victor DriveL1;
    private Victor DriveR1;
    private Victor DriveL2;
    private Victor DriveR2;
    private RobotDrive drive;
    private Joystick LeftJoy;
    private Joystick RightJoy;
    
    /**
     * This function is run when the robot is first started up and should be
     * used for any initialization code.
     */
    
    public void robotInit() {
           DriveL1 = new Victor(4);
           DriveR1 = new Victor(3);
           DriveL2 = new Victor(2);
           DriveR2 = new Victor(1);
           drive = new RobotDrive(DriveL1, DriveL2, DriveR1, DriveR2);
           drive.setInvertedMotor(MotorType.kFrontLeft,true);
           drive.setInvertedMotor(MotorType.kFrontRight,true);
           drive.setInvertedMotor(MotorType.kRearLeft,true);
           drive.setInvertedMotor(MotorType.kRearRight,true);
        
           LeftJoy = new Joystick(1);
           RightJoy = new Joystick(2);
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
        drive.tankDrive(LeftJoy, RightJoy);
    }
    
    /**
     * This function is called periodically during test mode
     */
    public void testPeriodic() {
    
    }
    
}
