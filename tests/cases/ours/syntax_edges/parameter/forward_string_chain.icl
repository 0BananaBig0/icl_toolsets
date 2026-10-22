Module EdgeParameterForwardStringChain {
  Parameter FULL = $PREFIX, $SUFFIX;
  LocalParameter SUFFIX = "_tail";
  Parameter PREFIX = $ROOT;
  Parameter ROOT = "head";
  Attribute label = $FULL;
}
