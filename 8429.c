int map_chroma_pred_mode(int intra_chroma_pred_mode, int IntraPredMode)
{
  if (intra_chroma_pred_mode==4) {
    return IntraPredMode;
  }
  else {
    static const enum IntraPredMode IntraPredModeCCand[4] = {
      INTRA_PLANAR,
      INTRA_ANGULAR_26, // vertical
      INTRA_ANGULAR_10, // horizontal
      INTRA_DC
    };

    int IntraPredModeC = IntraPredModeCCand[intra_chroma_pred_mode];
    if (IntraPredModeC == IntraPredMode) {
      return INTRA_ANGULAR_34;
    }
    else {
      return IntraPredModeC;
    }
  }
}