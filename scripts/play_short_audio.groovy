/**
 * binding:
 * - tracks[] = track names
 */

def mpc = "/usr/bin/mpc -h localhost "

(mpc + "stop").execute()
(mpc + "clear").execute()
tracks.each {
    mcApi.logger().info("mpc adding: ${it}");
    (mpc + "add ${it}").execute()
}
(mpc + "repeat off").execute()
res = (mpc + "playlist").execute().text
mcApi.logger().info("mpc: " + res);
(mpc + "play").execute().text
res = (mpc + "current").execute().text
mcApi.logger().info("Now playing " + res);
