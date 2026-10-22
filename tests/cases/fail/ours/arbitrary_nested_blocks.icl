// What the nested-block rule actually accepts, recorded because it is wider
// than the standard.
//
// scanInterfaceChain_item is attribute_def | scanInterfacePort_def |
// defaultLoad_def — it does not include scanInterfaceChain_def, so a Chain
// inside a Chain is not ICL. Same for a Reg inside a Reg, an Enum inside an
// Enum, a port block inside a port block.
//
// This formatter accepts all of them anyway, and it is one decision rather
// than four. The grammar has one rule for a named block:
//
//   attr_name postfix '{' attr_body '}'
//
// and attr_body contains that same rule again, so any keyword can open a
// block anywhere a block can open. Rejecting `Chain { Chain {` would mean a
// second, Chain-specific copy of attr_body — the two lists would then have to
// be kept in step, which is the mistake this project keeps making (§5.3).
//
// The trade is the same one made for `Enum E { }`: accepting a shape the
// standard excludes costs nothing, while rejecting a real file costs the
// whole file. Module is the one exception, and it is structural: module_def
// appears only in iclSource_items, so a Module inside a Module is rejected.
Module m {
  ScanInterface si {
    Chain outer {
      Chain inner {
        Port SI;
      }
    }
  }
  Reg r1 {
    Reg r2 {
      Width 4;
    }
  }
  Enum e1 {
    Enum e2 {
      a = 1;
    }
  }
  ScanInPort p1 {
    ScanInPort p2 {
      Source s;
    }
  }
}

// Reclassified: IEEE §6.5.3 permits only enum_symbol = concat_number;
// in an Enum. Nesting Enum e2 inside Enum e1 is not an extension in our sources.
