#ifndef SYNERGY_I2C20_H
  #define SYNERGY_I2C20_H


extern const transfer_instance_t   DTC_I2C0_TX_transfer_instance;
extern const transfer_instance_t   DTC_I2C0_RX_transfer_instance;
extern const i2c_master_instance_t I2C0_interface_instance;
extern riic_instance_ctrl_t        I2C0_control_block;
extern const riic_extended_cfg     I2C0_extended_configuration;
extern sf_i2c_bus_t                I2C0_bus;

#endif



