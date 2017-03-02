package c8y.lx.driver;

import c8y.Hardware;

public interface HardwareProvider {
    
    String UNKNOWN = "unknown";
    
    Hardware getHardware();

}
