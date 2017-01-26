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

    public static ManagedObjectRepresentation create(
            String id,
            String type,
            Platform platform,
            ManagedObjectRepresentation parent,
            Hardware hardware,
            OperationExecutor[] supportedOperations,
            Object... fragments
    ) {
        log.debug("creating managed object {} of type {}", id, type);

        ManagedObjectRepresentation device = new ManagedObjectRepresentation();

        device.setType(type);
        device.setName(id);

        for (OperationExecutor operation : supportedOperations) {
            log.info("registering supported operation type {} for {}", operation.supportedOperationType(), id);

            OpsUtil.addSupportedOperation(device, operation.supportedOperationType());
        }

        if (hardware != null) {
            device.set(hardware);
        }

        for (Object fragment : fragments) {
            if (fragment == null) {
                continue;
            }

            device.set(fragment);
        }

        DeviceManagedObject deviceManagedObject = new DeviceManagedObject(platform);
        ID externalId = DeviceManager.buildExternalId(parent, device, id);
        boolean wasCreated = deviceManagedObject.createOrUpdate(device, externalId, parent.getId());

        if (wasCreated) {
	        log.debug("created managed object {} of type {} with external id of {} and global id {}", id, type, externalId, device.getId().getValue());
        }

        return device;
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
