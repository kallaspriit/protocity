package com.stagnationlab.c8y.driver.services;

import java.util.ArrayList;
import java.util.List;

import lombok.extern.slf4j.Slf4j;

import com.cumulocity.model.ID;
import com.cumulocity.model.idtype.GId;
import com.cumulocity.rest.representation.inventory.ManagedObjectRepresentation;
import com.cumulocity.sdk.client.Platform;

import c8y.Hardware;
import c8y.lx.driver.DeviceManagedObject;
import c8y.lx.driver.OperationExecutor;
import c8y.lx.driver.OpsUtil;

@Slf4j
public class DeviceManager {

    public static ManagedObjectRepresentation createOrUpdate(
            String id,
            String type,
            Platform platform,
            ManagedObjectRepresentation parent,
            Hardware hardware,
            OperationExecutor[] supportedOperations,
            Object... fragments
    ) {
        log.debug("creating/updating managed object {} of type {}", id, type);

        ManagedObjectRepresentation device = new ManagedObjectRepresentation();

        device.setType(type);
        device.setName(id);

        if (hardware != null) {
            device.set(hardware);
        }

	    for (OperationExecutor operation : supportedOperations) {
		    log.debug("registering supported operation type {} for {}", operation.supportedOperationType(), id);

		    OpsUtil.addSupportedOperation(device, operation.supportedOperationType());
	    }

	    DeviceManagedObject deviceManagedObject = new DeviceManagedObject(platform);
	    ID externalId = DeviceManager.buildExternalId(parent, device, id);
	    GId globalId = deviceManagedObject.tryGetBinding(externalId);

	    log.debug("global id of '{}': {}", id, globalId == null ? "n/a" : globalId.getValue());

	    ManagedObjectRepresentation existingDevice = platform.getInventoryApi().get(globalId);

	    if (existingDevice != null) {
	    	log.debug("returning existing device for {} ()", id, globalId.getValue());

	    	return existingDevice;
	    }

	    // add fragments to a new device
        for (Object fragment : fragments) {
            if (fragment == null) {
                continue;
            }

            device.set(fragment);
        }

        boolean wasCreated = deviceManagedObject.createOrUpdate(device, externalId, parent.getId());

        log.debug("{} managed object {} of type {} with external id of {} and global id {}", wasCreated ? "created" : "updated", id, type, externalId, device.getId().getValue());

        return device;
    }

    public static ID buildExternalId(ManagedObjectRepresentation parent, ManagedObjectRepresentation child, String id) {
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
