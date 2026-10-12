// ClearRange.java: args are pairs <start> <end> (hex, end inclusive). Clears code units in each range (functions stay).
// Use after DeleteFunctions on a false split whose stale instructions overlap the real flow, then GrowToRange the owner.
import ghidra.app.script.GhidraScript;
public class ClearRange extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        for (int i = 0; i + 1 < a.length; i += 2) {
            clearListing(toAddr(Long.decode(a[i])), toAddr(Long.decode(a[i + 1])));
            println("ClearRange: " + a[i] + ".." + a[i + 1]);
        }
    }
}
