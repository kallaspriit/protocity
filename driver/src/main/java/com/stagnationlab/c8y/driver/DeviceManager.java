package com.stagnationlab.c8y.driver;

import c8y.Hardware;
import c8y.lx.driver.DeviceManagedObject;
import c8y.lx.driver.OperationExecutor;
import c8y.lx.driver.OpsUtil;
import com.cumulocity.model.ID;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.sdk.client.Platform;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class DeviceManager {

    private static final Logger log = LoggerFactory.getLogger(DeviceManager.class);

    public static ManagedObjectRepresentation createChild(
            String id,
            String type,
            Platform platform,
            ManagedObjectRepresentation parent,
            Hardware hardware,
            OperationExecutor[] supportedOperations,
            Object sensorFragment
    ) {
        log.info("creating child managed object with id '" + id + "' of type '" + type + "'");

        ManagedObjectRepresentation child = new ManagedObjectRepresentation();
        child.set(hardware);
        child.setType(type);
        child.setName(child.getType() + " " + id);

        for (OperationExecutor operation : supportedOperations) {
            log.info("registering supported operation type '" + operation.supportedOperationType() + "'");

            OpsUtil.addSupportedOperation(child, operation.supportedOperationType());
        }

        if (sensorFragment != null) {
            child.set(sensorFragment);
        }

        DeviceManagedObject deviceManagedObject = new DeviceManagedObject(platform);
        ID externalId = DeviceManager.buildExternalId(parent, child, id);
        deviceManagedObject.createOrUpdate(child, externalId, parent.getId());

        return child;
    }

    public static ManagedObjectRepresentation createChild(
            String id,
            String type,
            Platform platform,
            ManagedObjectRepresentation parent,
            Hardware hardware,
            OperationExecutor[] supportedOperations
    ) {
        return createChild(id, type, platform, parent, hardware, supportedOperations, null);
    }

    private static ID buildExternalId(ManagedObjectRepresentation parent, ManagedObjectRepresentation child, String id) {
        return new ID(
                parent.get(Hardware.class).getSerialNumber() +
                "-" + child.get(Hardware.class).getSerialNumber() +
                "-" + id
        );
    }

}
