/**
 * Returns true, if AlarmArmed variable is not "TRUE", otherwise returns false.
 */

def alarmArmedVar = mcApi.variable().get("AlarmArmed")
if (alarmArmedVar != null) {
    String alarmArmedValue = alarmArmedVar.getValue() //value as String
    if (alarmArmedValue != null && alarmArmedValue == "TRUE") {
        return false;
    }
}
return true;