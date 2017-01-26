package com.stagnationlab.c8y.driver.devices.etherio;

import com.stagnationlab.c8y.driver.devices.AbstractMonitoringSensor;
import com.stagnationlab.c8y.driver.measurements.MonitoringMeasurement;
import com.stagnationlab.etherio.Commander;

public class EtherioMonitoringSensor extends AbstractMonitoringSensor {

	private final Commander commander;

	private final static int TOTAL_MEMORY_BYTES = 16384; // approximately

	public EtherioMonitoringSensor(String id, Commander commander) {
		super(id);

		this.commander = commander;
	}

	@Override
	public void start() {
		setInterval(() -> {
			commander.sendCommand("memory").thenAccept(
					commandResponse -> {
						int freeMemoryBytes = commandResponse.response.getInt(0);

						MonitoringMeasurement monitoringMeasurement = new MonitoringMeasurement(
								TOTAL_MEMORY_BYTES,
								freeMemoryBytes
						);

						reportMeasurement(monitoringMeasurement);
					}
			);




		}, 5000);
	}
}
