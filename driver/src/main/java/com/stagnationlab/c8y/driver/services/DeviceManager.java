package com.stagnationlab.c8y.driver.services;

import c8y.Hardware;
import c8y.lx.driver.DeviceManagedObject;
import c8y.lx.driver.OperationExecutor;
import c8y.lx.driver.OpsUtil;
import com.cumulocity.model.ID;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.sdk.client.Platform;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

public class DeviceManager {

    private static final Logger log = LoggerFactory.getLogger(DeviceManager.class);

    public static ManagedObjectRepresentation createChild(
            String id,
            String type,
            Platform platform,
            ManagedObjectRepresentation parent,
            Hardware hardware,
            OperationExecutor[] supportedOperations,
            Object... fragments
    ) {
        log.info("creating child managed object with id '" + id + "' of type '" + type + "'");

        ManagedObjectRepresentation child = new ManagedObjectRepresentation();
        child.setType(type);
        child.setName(id);

        for (OperationExecutor operation : supportedOperations) {
            log.info("registering supported operation type '" + operation.supportedOperationType() + "'");

            OpsUtil.addSupportedOperation(child, operation.supportedOperationType());
        }

        if (hardware != null) {
            child.set(hardware);
        }

        for (Object fragment : fragments) {
            if (fragment == null) {
                continue;
            }

            child.set(fragment);
        }

        DeviceManagedObject deviceManagedObject = new DeviceManagedObject(platform);
        ID externalId = DeviceManager.buildExternalId(parent, child, id);
        deviceManagedObject.createOrUpdate(child, externalId, parent.getId());

        return child;
    }

    private static ID buildExternalId(ManagedObjectRepresentation parent, ManagedObjectRepresentation child, String id) {
        List<String> tokens = new ArrayList<>();
        Hardware parentHardware = parent.get(Hardware.class);
        Hardware childHardware = child.get(Hardware.class);

        if (parentHardware != null) {
            tokens.add(parentHardware.getSerialNumber());
        }

        if (childHardware != null) {
            tokens.add(childHardware.getSerialNumber());
        }

        tokens.add(child.getType());
        tokens.add(id);

        String identifier = String.join("-", tokens);

        return new ID(identifier);
    }

}
