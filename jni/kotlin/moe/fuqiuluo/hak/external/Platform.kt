package moe.fuqiuluo.hak.external

object ProcessUtils {
    data class ProcStat(
        val pid: Int,
        val comm: String,
        val state: Char,
        val ppid: Int
    )

    external fun getPid(packageName: String): Int

    external fun getPidList(): List<Int>

    external fun getProcessList(): List<ProcStat>
}