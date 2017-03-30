def uid = "AlarmArmed"
def value = "1"

mcApi.logger().info("Setting " + uid + "=" + value);

def sensor = mcApi.uidTag().getByUid(uid).getResource()
if (sensor) {
    sensor.setValue(value)
    mcApi.sensor().sendPayload(sensor);
}
else
    mcApi.logger().info("UID=" + uid + " not found");

