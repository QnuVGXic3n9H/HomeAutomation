/**
 * binding:
 * - newAlarmLevel = 0|1|2
 */

def mpc = "/usr/bin/mpc -h localhost "

//Get current alarm level (variable in the repository)
def alarmLevelVar = mcApi.variable().get("AlarmLevel")
def alarmLevelVarValue = alarmLevelVar.getValue() //value as String
if (alarmLevelVarValue == null || !alarmLevelVarValue.isInteger()) {
    alarmLevelVarValue = "0"
}
//sanity checks
alarmLevelVarValue = alarmLevelVarValue.toInteger()
if (alarmLevelVarValue < 0)
    alarmLevelVarValue = 0
if (alarmLevelVarValue > 2)
    alarmLevelVarValue = 2
mcApi.logger().info("Current AlarmLevel value(" + alarmLevelVarValue + ") new value(" + newAlarmLevel + ")");


if (newAlarmLevel == 0) {
    if(alarmLevelVarValue>0) {
        //Stop playback
        (mpc + "stop").execute()
        (mpc + "clear").execute()

        mcApi.logger().info("Playback stopped");
    }

} else {
    if (newAlarmLevel > alarmLevelVarValue) {
        def track
        switch (newAlarmLevel) {
            case 1:
                track = "DOGS_ANGRY_BARKING.mp4";
                break

            case 2:
                track = "220424__nahlin83__siren.wav"
        }

        (mpc + "stop").execute()
        (mpc + "clear").execute()
        (mpc + "add " + track).execute()
        (mpc + "repeat on").execute()
        res = (mpc + "playlist").execute().text
        mcApi.logger().info("mpc: " + res);
        (mpc + "play").execute().text
        res = (mpc + "current").execute().text
        mcApi.logger().info("Now playing " + res);
    }
}

if (alarmLevelVarValue != newAlarmLevel) {
    //Change variable in the repository
    alarmLevelVar.setValue(newAlarmLevel.toString()) //Should be in the form of String
    alarmLevelVar.save()
}

//reset timer to stop playback

def filters = [:];
filters.put("name", "Stop Playback Timer");
def timer = mcApi.timer().get(filters);

def timerIds = [timer.getId()];
mcApi.timer().disable(timerIds)

if (newAlarmLevel > 0) {
    mcApi.timer().enable(timerIds)

    mcApi.logger().info("Reset timer to stop playback");
}