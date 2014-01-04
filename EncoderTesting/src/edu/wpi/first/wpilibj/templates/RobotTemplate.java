/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpilibj.templates;
import edu.wpi.first.wpilibj.Encoder;
import edu.wpi.first.wpilibj.Victor;
import edu.wpi.first.wpilibj.IterativeRobot;
import edu.wpi.first.wpilibj.livewindow.LiveWindow;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

/**
 * The VM is configured to automatically run this class, and to call the
 * functions corresponding to each mode, as described in the IterativeRobot
 * documentation. If you change the name of this class or the package after
 * creating this project, you must also update the manifest file in the resource
 * directory.
 */
public class RobotTemplate extends IterativeRobot {
    private static final int MotorShooter1 = 7;
    private static final int DIOshooterEncoder1A = 7;
    private static final int DIOshooterEncoder1B = 6; 
    
    private SmartEncoder se;
    
    private Encoder shooterEncoder1;
    private Victor shooterMotor1;
    
    
    /**
     * This function is run when the robot is first started up and should be
     * used for any initialization code.
     */
    public void robotInit() {
        shooterEncoder1 = new Encoder(DIOshooterEncoder1A,DIOshooterEncoder1B);
        shooterEncoder1.setDistancePerPulse(60.0 / 64); //pulse per revolution, multiplied by 60 to RPM?
        shooterEncoder1.start();
        
        shooterMotor1 = new Victor(MotorShooter1);
        
        se = new SmartEncoder();
        se.start();
        
        LiveWindow.addSensor("Shooter", "Encoder 1", shooterEncoder1);
        LiveWindow.addActuator("Shooter", "Motor 1", shooterMotor1);
        LiveWindow.addSensor("Shooter", "Smart Encoder", se);
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
        
    }
    
    /**
     * This function is called periodically during test mode
     */
    public void testPeriodic() {
        LiveWindow.run();
        SmartDashboard.putNumber("Encoder RPM", shooterEncoder1.getRate());
    }
    
}
