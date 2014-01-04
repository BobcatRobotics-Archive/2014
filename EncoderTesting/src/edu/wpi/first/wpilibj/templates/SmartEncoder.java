/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.wpi.first.wpilibj.templates;

import edu.wpi.first.wpilibj.DigitalModule;
import edu.wpi.first.wpilibj.I2C;
import edu.wpi.first.wpilibj.livewindow.LiveWindowSendable;
import edu.wpi.first.wpilibj.tables.ITable;

/**
 *
 * @author schrod
 */
public class SmartEncoder extends Thread implements LiveWindowSendable {
    
    private static final int I2CEncoderAddress = 0xB1;
    private static final float I2CEncoderScale = 28.7224f;
    
    private static class command {
        static byte ReadPeriodBasedSpeed    = 0x01;
        static byte ReadFrequencyBasedSpeed = 0x02;
        static byte ReadDistance            = 0x03;
        static byte ReadAll                 = 0x04;
        static byte ResetDistance           = 0x42;
    }
    
    private double m_distancePerPulse = 1.0f;		// distance of travel for each encoder tick

    private short PeriodBasedSpeed;
    private short FrequencyBasedSpeed;
    private short Distance;
    
    private final I2C I2CEncoder;
            
    SmartEncoder() {
        I2CEncoder = new I2C(DigitalModule.getInstance(0),I2CEncoderAddress);   
        PeriodBasedSpeed = 0;
        FrequencyBasedSpeed = 0;
        Distance = 0;
    }
    
    public void run() {
        byte[] readData = new byte[6];
        while(true) {
            
            if(!I2CEncoder.read(command.ReadAll, readData.length, readData)) {
                PeriodBasedSpeed = (short)((readData[0]<<8) + readData[1]);
                FrequencyBasedSpeed = (short)((readData[2]<<8) + readData[3]);
                Distance = (short)((readData[4]<<8) + readData[5]);
            }
            
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
            }
        }
    }
    
    public double getPeriodBasedSpeed() {
        return PeriodBasedSpeed * (60/64);
    }
    
    public double getFrequencyBasedSpeed() {
        return FrequencyBasedSpeed * I2CEncoderScale;
    }
    
    
    public double getDistance() {
        return Distance * m_distancePerPulse;
    }
    
    public void reset() {
        byte[] cmd = new byte[1];
        cmd[0] = command.ResetDistance;
        I2CEncoder.transaction(cmd, 1, null, 0);
        Distance = 0;
    }
    
    /*
     * Live Window code, only does anything if live window is activated.
     */
    public String getSmartDashboardType(){
        return "Smart Encoder";
    }
    private ITable m_table;
    
    /**
     * {@inheritDoc}
     */
    public void initTable(ITable subtable) {
        m_table = subtable;
        updateTable();
    }
    
    /**
     * {@inheritDoc}
     */
    public ITable getTable(){
        return m_table;
    }
    
    /**
     * {@inheritDoc}
     */
    public void updateTable() {
        if (m_table != null) {
            m_table.putNumber("Period Speed", getPeriodBasedSpeed());
            m_table.putNumber("Frequency Speed", getFrequencyBasedSpeed());
            m_table.putNumber("Distance", getDistance());
        }
    }

    /**
     * {@inheritDoc}
     */
    public void startLiveWindowMode() {}
    
    /**
     * {@inheritDoc}
     */
    public void stopLiveWindowMode() {}
}
