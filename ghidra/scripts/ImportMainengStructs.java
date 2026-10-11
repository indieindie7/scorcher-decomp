// Imports Zyrinx engine struct layouts from MAINENG.ASC (shipped on the Scorcher Saturn disc)
// into the current program's data type manager, under category /Scorcher.
// Format: constants "name value"; then blocks "Struct 0, size, 1" followed by
// "Struct_field offset, size, typecode[, typename]". Typecodes: 1 struct/group, 2 scalar,
// 256 routine ptr, 257 ptr to struct, 258 generic ptr, 513 ptr to ptr to struct, 514 ptr to ptr.
// Usage: analyzeHeadless ... -process LORT.BIN -postScript ImportMainengStructs.java <path to MAINENG.ASC>
//@category Scorcher
import ghidra.app.script.GhidraScript;
import ghidra.program.model.data.*;
import java.io.*;
import java.util.*;

public class ImportMainengStructs extends GhidraScript {
    DataTypeManager dtm;
    CategoryPath cat = new CategoryPath("/Scorcher");
    Map<String, StructureDataType> structs = new LinkedHashMap<>();

    StructureDataType getStruct(String name, int size) {
        StructureDataType s = structs.get(name);
        if (s == null) {
            s = new StructureDataType(cat, name, Math.max(size, 0), dtm);
            structs.put(name, s);
        } else if (size > s.getLength()) {
            s.growStructure(size - s.getLength());
        }
        return s;
    }

    DataType scalar(int size) {
        switch (size) {
            case 1: return ByteDataType.dataType;
            case 2: return ShortDataType.dataType;
            case 4: return IntegerDataType.dataType;
            default:
                if (size % 4 == 0) return new ArrayDataType(IntegerDataType.dataType, size / 4, 4);
                return new ArrayDataType(ByteDataType.dataType, size, 1);
        }
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) { printerr("need path to MAINENG.ASC"); return; }
        dtm = currentProgram.getDataTypeManager();
        DataType voidp = new PointerDataType(VoidDataType.dataType, 4, dtm);
        FunctionDefinitionDataType routDef = new FunctionDefinitionDataType(cat, "EngineRout", dtm);
        routDef.setReturnType(VoidDataType.dataType);
        DataType routp = new PointerDataType(routDef, 4, dtm);

        List<String> lines = new ArrayList<>();
        try (BufferedReader r = new BufferedReader(new FileReader(args[0]))) {
            String l; while ((l = r.readLine()) != null) lines.add(l);
        }
        int consts = 0, fields = 0;
        String cur = null;
        StringBuilder enumNote = new StringBuilder();
        for (int ln = 0; ln < lines.size(); ln++) {
            String line = lines.get(ln).trim();
            if (line.isEmpty()) { cur = null; continue; }
            String[] head = line.split("\\s+", 2);
            if (head.length < 2) continue;
            String name = head[0];
            String[] cols = head[1].split(",");
            for (int i = 0; i < cols.length; i++) cols[i] = cols[i].trim();
            if (cols.length == 1) {   // constant
                consts++;
                enumNote.append(name).append('=').append(cols[0]).append('\n');
                continue;
            }
            int off = Integer.parseInt(cols[0]);
            int size = Integer.parseInt(cols[1]);
            int tc = Integer.parseInt(cols[2]);
            String tname = cols.length > 3 ? cols[3] : null;
            if (tc == 1 && cur == null) { getStruct(name, size); cur = name; continue; }
            if (tc == 1) continue;            // inline group header; its fields follow
            if (cur == null) continue;        // stray field line
            StructureDataType s = getStruct(cur, off + size);
            DataType dt;
            switch (tc) {
                case 2: dt = scalar(size); break;
                case 256: dt = routp; break;
                case 257: dt = new PointerDataType(getStruct(tname, 0), 4, dtm); break;
                case 513: dt = new PointerDataType(new PointerDataType(getStruct(tname, 0), 4, dtm), 4, dtm); break;
                case 514: dt = new PointerDataType(voidp, 4, dtm); break;
                default: dt = voidp; break;
            }
            String fname = name;
            if (fname.startsWith(cur)) fname = fname.substring(cur.length());
            if (fname.startsWith("_")) fname = fname.substring(1);
            if (fname.isEmpty()) fname = name;
            try {
                s.replaceAtOffset(off, dt, size, fname, "line " + (ln + 1));
                fields++;
            } catch (Exception e) {
                printerr(cur + "." + fname + " @" + off + ": " + e.getMessage());
            }
        }
        int added = 0;
        for (StructureDataType s : structs.values()) { dtm.addDataType(s, DataTypeConflictHandler.REPLACE_HANDLER); added++; }
        // keep the constants as a program-level comment block for reference
        File out = new File(new File(args[0]).getParentFile(), "MAINENG_constants.txt");
        try (PrintWriter w = new PrintWriter(out)) { w.print(enumNote); }
        println("structs " + added + ", fields " + fields + ", constants " + consts + " -> " + out);
    }
}
