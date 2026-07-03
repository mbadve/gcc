/* { dg-do compile } */
/* { dg-additional-options "-march=rv32i_xmipscmov_xmipsexectl_xmipscbom_xmipsmginv_xmipscbop_xmipslsp_xmipsuarchdis_xmipsfeatdis_xmipsdebugbase_xmipswfe_xmipstrig_xmipsrtmt_xmipssecarch_xmipsembpma_xmipsclicvirt -mabi=ilp32" } */

int main ()
{
  return 0;
}

/* { dg-final { scan-assembler {.attribute arch, "rv32i2p1_xmipscbom1p0_xmipscbop1p0_xmipsclicvirt1p0_xmipscmov1p0_xmipsdebugbase1p0_xmipsembpma1p0_xmipsexectl1p0_xmipsfeatdis1p0_xmipslsp1p0_xmipsmginv1p0_xmipsrtmt1p0_xmipssecarch1p0_xmipstrig1p0_xmipsuarchdis1p0_xmipswfe1p0"} } } */

