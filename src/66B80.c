/*
 * A display-list printer; its output was compiled out of the retail build.
 */
#include "common.h"

/* Unreferenced debug strings at the start of this file's .rodata: kept as asm. */
INCLUDE_RODATA("asm/nonmatchings/66B80", D_800ADAC0);

s32 dl_cmd_print_vertex();
s32 dl_cmd_print_polygon();
s32 dl_cmd_print_matrix();
s32 dl_cmd_print_tex_offsets();
s32 dl_cmd_print_dp_block();
s32 dl_cmd_print_viewport();
s32 dl_cmd_print_display_list();
s32 dl_cmd_print_named();
s32 dl_cmd_print_prim_colour();
s32 dl_cmd_print_colour();
s32 dl_cmd_print_dma_offsets();
s32 dl_cmd_print_move_word();
s32 dl_cmd_print_raw();
s32 dl_cmd_print_other_mode_l();
s32 dl_cmd_print_geometry_mode();

/*
 * JFG: the display-list printer: walks `count` commands and hands each to its printer by opcode (the signed
 * F3D opcodes in a second switch), stepping by the size each printer returns. The prints themselves were
 * compiled out of the retail build.
 */
void dl_dump_commands(u8 *dl, UNUSED s32 arg1, s32 count) {
    u8 *cmd;
    s32 i;

    cmd = dl;
    for (i = 0; i < count; i++) {
        switch (*cmd) {
            case 4:
                cmd += dl_cmd_print_vertex(cmd);
                break;
            case 5:
                cmd += dl_cmd_print_polygon(cmd);
                break;
            case 0:
                cmd += dl_cmd_print_raw(cmd, "G_SPNOOP");
                break;
            case 1:
                cmd += dl_cmd_print_matrix(cmd);
                break;
            case 2:
                cmd += dl_cmd_print_tex_offsets(cmd, "G_TEXDMA");
                break;
            case 3:
                cmd += dl_cmd_print_viewport(cmd);
                break;
            case 6:
                cmd += dl_cmd_print_display_list(cmd);
                break;
            case 7:
                cmd += dl_cmd_print_dp_block(cmd);
                break;
            case 8:
                cmd += dl_cmd_print_raw(cmd, "G_RESERVED3");
                break;
            case 9:
                cmd += dl_cmd_print_raw(cmd, "G_RESERVED4");
                break;
            case 0xC0:
                cmd += dl_cmd_print_named(cmd, "G_NOOP", "gDPNoOp");
                break;
            case 0xFF:
                cmd += dl_cmd_print_raw(cmd, "G_SETCIMG");
                break;
            case 0xFE:
                cmd += dl_cmd_print_raw(cmd, "G_SETZIMG");
                break;
            case 0xFD:
                cmd += dl_cmd_print_raw(cmd, "G_SETTIMG");
                break;
            case 0xFC:
                cmd += dl_cmd_print_raw(cmd, "G_SETCOMBINE");
                break;
            case 0xFB:
                cmd += dl_cmd_print_colour(cmd, "G_SETENVCOLOR", "gDPSetEnvColor");
                break;
            case 0xFA:
                cmd += dl_cmd_print_prim_colour(cmd);
                break;
            case 0xF9:
                cmd += dl_cmd_print_colour(cmd, "G_SETBLENDCOLOR", "gDPSetBlendColor");
                break;
            case 0xF8:
                cmd += dl_cmd_print_colour(cmd, "G_SETFOGCOLOR", "gDPSetFogColor");
                break;
            case 0xF7:
                cmd += dl_cmd_print_colour(cmd, "G_SETFILLCOLOR", "gDPSetFillColor");
                break;
            case 0xF6:
                cmd += dl_cmd_print_raw(cmd, "G_FILLRECT");
                break;
            case 0xF5:
                cmd += dl_cmd_print_raw(cmd, "G_SETTILE");
                break;
            case 0xF4:
                cmd += dl_cmd_print_raw(cmd, "G_LOADTILE");
                break;
            case 0xF3:
                cmd += dl_cmd_print_raw(cmd, "G_LOADBLOCK");
                break;
            case 0xF2:
                cmd += dl_cmd_print_raw(cmd, "G_SETTILESIZE");
                break;
            case 0xF0:
                cmd += dl_cmd_print_raw(cmd, "G_LOADTLUT");
                break;
            case 0xEF:
                cmd += dl_cmd_print_raw(cmd, "G_RDPSETOTHERMODE");
                break;
            case 0xEE:
                cmd += dl_cmd_print_raw(cmd, "G_SETPRIMDEPTH");
                break;
            case 0xED:
                cmd += dl_cmd_print_raw(cmd, "G_SETSCISSOR");
                break;
            case 0xEC:
                cmd += dl_cmd_print_raw(cmd, "G_SETCONVERT");
                break;
            case 0xEB:
                cmd += dl_cmd_print_raw(cmd, "G_SETKEYR");
                break;
            case 0xEA:
                cmd += dl_cmd_print_raw(cmd, "G_SETKEYGB");
                break;
            case 0xE9:
                cmd += dl_cmd_print_named(cmd, "G_RDPFULLSYNC", "gDPFullSync");
                break;
            case 0xE8:
                cmd += dl_cmd_print_named(cmd, "G_RDPTILESYNC", "gDPTileSync");
                break;
            case 0xE7:
                cmd += dl_cmd_print_named(cmd, "G_RDPPIPESYNC", "gDPPipeSync");
                break;
            case 0xE6:
                cmd += dl_cmd_print_named(cmd, "G_RDPLoadSYNC", "gDPLoadSync");
                break;
            case 0xE5:
                cmd += dl_cmd_print_raw(cmd, "G_TEXRECTFLIP");
                break;
            case 0xE4:
                cmd += dl_cmd_print_raw(cmd, "G_TEXRECT");
                break;
            default:
                switch ((s8) *cmd) {
                    case -65:
                        cmd += dl_cmd_print_dma_offsets(cmd, "G_DMAOFFSETS");
                        break;
                    case -66:
                        cmd += dl_cmd_print_raw(cmd, "G_CULLDL");
                        break;
                    case -67:
                        cmd += dl_cmd_print_raw(cmd, "G_POPMTX");
                        break;
                    case -68:
                        cmd += dl_cmd_print_move_word(cmd);
                        break;
                    case -69:
                        cmd += dl_cmd_print_raw(cmd, "G_TEXTURE");
                        break;
                    case -70:
                        cmd += dl_cmd_print_raw(cmd, "G_SETOTHERMODE_H");
                        break;
                    case -71:
                        cmd += dl_cmd_print_other_mode_l(cmd, "G_SETOTHERMODE_L");
                        break;
                    case -72:
                        cmd += dl_cmd_print_raw(cmd, "G_ENDDL");
                        break;
                    case -73:
                        cmd += dl_cmd_print_geometry_mode(cmd, "G_SETGEOMETRYMODE", "gSPSetGeometryMode");
                        break;
                    case -74:
                        cmd += dl_cmd_print_geometry_mode(cmd, "G_CLEARGEOMETRYMODE", "gSPClearGeometryMode");
                        break;
                    case -75:
                        cmd += dl_cmd_print_raw(cmd, "G_LINE3D");
                        break;
                    case -76:
                        cmd += dl_cmd_print_raw(cmd, "G_PERSPNORMALIZE");
                        break;
                    case -77:
                        cmd += dl_cmd_print_raw(cmd, "G_RDPHALF_1");
                        break;
                    case -78:
                        cmd += dl_cmd_print_raw(cmd, "G_RDPHALF_2");
                        break;
                    case -79:
                        cmd += dl_cmd_print_raw(cmd, "G_RDPHALF_CONT");
                        break;
                    default:
                        cmd += dl_cmd_print_raw(cmd, "UNKNOWN COMMAND");
                        break;
                }
                break;
        }
    }
}

/* JFG: unpack a two-word command: the top byte, the next byte, the low half and the second word. */
void dl_cmd_unpack_bbh(void *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (((u32 *) arg0)[0] >> 24) & 0xFF;
    *d = (((u32 *) arg0)[0] >> 16) & 0xFF;
    *c = ((u32 *) arg0)[0] & 0xFFFF;
    *b = ((u32 *) arg0)[1];
}

/* JFG: unpack a two-word command: the top byte, the middle half, the low byte and the second word. */
void dl_cmd_unpack_bhb(u32 *arg0, s32 *a, s32 *b, s32 *c, s32 *d) {
    *a = (arg0[0] >> 24) & 0xFF;
    *b = (arg0[0] >> 8) & 0xFFFF;
    *c = arg0[0] & 0xFF;
    *d = arg0[1];
}

/* Prints a vertex command (0x80066620). */
s32 dl_cmd_print_vertex(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    stubbed_printf("%20s: gSPVertex(0x%x, 0x%x, %d, %d);\n", "G_VTX");
    stubbed_printf("%20s: gSPPolygon(0x%x, 0x%x, %d, %d);\n", "G_POL");
    stubbed_printf("GARBAGE");
    stubbed_printf("NOMTXMULT");
    stubbed_printf("MTXMULT");
    stubbed_printf("%20s: gSPMatrix2(0x%x, 0x%x, %d, %s, %d);\n", "G_MTX");
    stubbed_printf("%20s: gSPLoadTextureOffsets(%d, %08x);\n");
    stubbed_printf("%20s: gSPDPBlock(0x%x, 0x%x, %d);\n", "G_DP_BLOCK");
    stubbed_printf("%20s: gSPViewport(0x%x, 0x%x);\n", "G_MOVEMEM");
    stubbed_printf("%20s: gSPDisplayList(0x%x, 0x%x);\n", "G_DL");
    stubbed_printf("%20s: gSPBranchList(0x%x, 0x%x);\n", "G_DL");
    stubbed_printf("%20s: Unknown G_DL command\n", "G_DL");
    stubbed_printf("%20s: %s(0x%08x);\n");
    stubbed_printf("%20s: gDPSetPrimColor(0x%08x, %d, %d, %d, %d, %d, %d);\n", "G_SETPRIMCOLOR");
    stubbed_printf("%20s: %s(0x%08x, %2d, %2d, %2d, %2d);\n");
    stubbed_printf("%20s: gSPSetDMAOffsets(%08x, %08x)\n");
    return 8;
}

/* Prints a polygon command (0x80066650). */
s32 dl_cmd_print_polygon(void *arg0) {
    s32 a, b, c, d;
    s32 pad[4];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

/* Prints a matrix command (0x80066680). */
s32 dl_cmd_print_matrix(void *arg0) {
    s32 a, b, c, d;
    s32 pad[8];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

/* Prints a texture offsets command (0x800666B0). */
s32 dl_cmd_print_tex_offsets(s32 arg0, s32 arg1) {
    return 0x8;
}

/* Prints a DP block command (0x800666C0). */
s32 dl_cmd_print_dp_block(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

/* Prints a move-memory (viewport) command (0x800666F0). */
s32 dl_cmd_print_viewport(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

/* Prints a display list call or branch (0x80066720). */
s32 dl_cmd_print_display_list(void *arg0) {
    s32 a, b, c, d;
    s32 pad[2];
    dl_cmd_unpack_bbh(arg0, &a, &b, &c, &d);
    return 8;
}

/* Prints a command with no arguments by its name (0x80066750). */
s32 dl_cmd_print_named(s32 arg0, s32 arg1, s32 arg2) {
    return 0x8;
}

/* Prints a primitive colour command (0x80066764). */
s32 dl_cmd_print_prim_colour(s32 arg0) {
    return 0x8;
}

/* Prints a colour command (0x80066770). */
s32 dl_cmd_print_colour(s32 arg0, s32 arg1, s32 arg2) {
    return 0x8;
}

/*
 * JFG: 8 bytes long. Only the second argument is spilled: IDO homes the parameters a function never
 * reads, so the first one is tested (the test's body was compiled out).
 */
s32 dl_cmd_print_dma_offsets(s32 arg0, UNUSED s32 arg1) {
    if (arg0) {
    }
    return 8;
}

/*
 * JFG: the "move word" command: names the index (the middle half of the first word), or formats an
 * unknown one into a local buffer; the print of the command was compiled out. 8 bytes long.
 * (`if (name) {}` keeps the case blocks apart as in the ROM; name is set on every path.)
 */
s32 dl_cmd_print_move_word(u32 *cmd) {
    s32 a;
    s32 index;
    s32 offset;
    s32 data;
    char *name;
    char buffer[0x28];

    dl_cmd_unpack_bhb(cmd, &a, &offset, &index, &data);
    switch (index) {
        case 6:
            name = "G_MW_SEGMENT";
            break;
        case 4:
            name = "G_MW_CLIP";
            break;
        case 10:
            name = "G_MW_MTXOFFSET";
            break;
        case 12:
            name = "G_MW_POINTS";
            break;
        case 2:
            name = "G_MW_SPRITEMODE";
            break;
        case 8:
            name = "G_MW_FOG";
            break;
        default:
            sprintf(buffer, "UNKNOWN INDEX %d", index);
            name = buffer;
            break;
    }
    if (name) {}
    stubbed_printf("%20s: %s offset = %d, data = 0x%08x\n", "G_MOVEWORD", name, offset, data);
    return 8;
}

/* JFG: format the command's argument into a local buffer (the print of the command was compiled out);
   8 bytes long. */
s32 dl_cmd_print_raw(UNUSED s32 arg0, s32 arg1) {
    char buffer[0x50];
    UNUSED s32 pad[2];

    sprintf(buffer, "%20s", arg1);
    stubbed_printf("%s: 0x%08x  0x%08x\n");
    return 8;
}

/*
 * JFG: the "set other mode" command: the alpha compare, depth source, render mode (named when it is one of
 * the standard modes, else spelt out bit by bit) or blender; the prints were compiled out. 8 bytes long.
 */
s32 dl_cmd_print_other_mode_l(u32 *cmd, s32 arg1) {
    char buffer[0x50];
    UNUSED s32 pad;
    s32 type;
    s32 w1;

    type = (cmd[0] >> 8) & 0xFF;
    w1 = cmd[1];
    sprintf(buffer, "%20s", arg1);
    switch (type) {
        case 0:
            stubbed_printf("%20s: gSetAlphaCompare(%x)\n");
            break;
        case 2:
            stubbed_printf("%20s: gSetDepthSource(%x)\n");
            break;
        case 3:
            stubbed_printf("%20s: gSetRenderMode(");
            w1 &= 0xFFFF;
            if (w1 == 0x2078) {
                stubbed_printf("RM_AA_ZB_OPA_SURF");
            } else if (w1 == 0x2038) {
                stubbed_printf("RM_RA_ZB_OPA_SURF");
            } else if (w1 == 0x49D8) {
                stubbed_printf("RM_AA_ZB_XLU_SURF");
            } else if (w1 == 0x2D58) {
                stubbed_printf("RM_AA_ZB_OPA_DECAL");
            } else if (w1 == 0x2D18) {
                stubbed_printf("RM_RA_ZB_OPA_DECAL");
            } else if (w1 == 0x4DD8) {
                stubbed_printf("RM_AA_ZB_XLU_DECAL");
            } else if (w1 == 0x2478) {
                stubbed_printf("RM_AA_ZB_OPA_INTER");
            } else if (w1 == 0x45D8) {
                stubbed_printf("RM_AA_ZB_XLU_INTER");
            } else if (w1 == 0x7858) {
                stubbed_printf("RM_AA_ZB_XLU_LINE");
            } else if (w1 == 0x7F58) {
                stubbed_printf("RM_AA_ZB_DEC_LINE");
            } else if (w1 == 0x3078) {
                stubbed_printf("RM_AA_ZB_TEX_EDGE");
            } else if (w1 == 0x3478) {
                stubbed_printf("RM_AA_ZB_TEX_INTER");
            } else if (w1 == 0x2278) {
                stubbed_printf("RM_AA_ZB_SUB_SURF");
            } else if (w1 == 0x7B) {
                stubbed_printf("RM_AA_ZB_PCL_SURF");
            } else if (w1 == 0x2078) {
                stubbed_printf("RM_AA_ZB_OPA_TERR");
            } else if (w1 == 0x3078) {
                stubbed_printf("RM_AA_ZB_TEX_TERR");
            } else if (w1 == 0x2278) {
                stubbed_printf("RM_AA_ZB_SUB_TERR");
            } else if (w1 == 0x2048) {
                stubbed_printf("RM_AA_OPA_SURF");
            } else if (w1 == 0x2008) {
                stubbed_printf("RM_RA_OPA_SURF");
            } else if (w1 == 0x41C8) {
                stubbed_printf("RM_AA_XLU_SURF");
            } else if (w1 == 0x7048) {
                stubbed_printf("RM_AA_XLU_LINE");
            } else if (w1 == 0x7248) {
                stubbed_printf("RM_AA_DEC_LINE");
            } else if (w1 == 0x3048) {
                stubbed_printf("RM_AA_TEX_EDGE");
            } else if (w1 == 0x2248) {
                stubbed_printf("RM_AA_SUB_SURF");
            } else if (w1 == 0x4B) {
                stubbed_printf("RM_AA_PCL_SURF");
            } else if (w1 == 0x2048) {
                stubbed_printf("RM_AA_OPA_TERR");
            } else if (w1 == 0x3048) {
                stubbed_printf("RM_AA_TEX_TERR");
            } else if (w1 == 0x2248) {
                stubbed_printf("RM_AA_SUB_TERR");
            } else if (w1 == 0x2230) {
                stubbed_printf("RM_ZB_OPA_SURF");
            } else if (w1 == 0x4A50) {
                stubbed_printf("RM_ZB_XLU_SURF");
            } else if (w1 == 0x2E10) {
                stubbed_printf("RM_ZB_OPA_DECAL");
            } else if (w1 == 0x4E50) {
                stubbed_printf("RM_ZB_XLU_DECAL");
            } else if (w1 == 0x4B50) {
                stubbed_printf("RM_ZB_CLD_SURF");
            } else if (w1 == 0x4F50) {
                stubbed_printf("RM_ZB_OVL_SURF");
            } else if (w1 == 0x233) {
                stubbed_printf("RM_ZB_PCL_SURF");
            } else if (w1 == 0x4000) {
                stubbed_printf("RM_OPA_SURF");
            } else if (w1 == 0x4240) {
                stubbed_printf("RM_XLU_SURF");
            } else if (w1 == 0x7008) {
                stubbed_printf("RM_TEX_EDGE");
            } else if (w1 == 0x4340) {
                stubbed_printf("RM_CLD_SURF");
            } else if (w1 == 0x4203) {
                stubbed_printf("RM_PCL_SURF");
            } else if (w1 == 0x4340) {
                stubbed_printf("RM_ADD");
            } else if (w1 == 0x0) {
                stubbed_printf("RM_NOOP");
            } else if (w1 == 0x4040) {
                stubbed_printf("RM_VISCVG");
            } else if (w1 == 0x0) {
                stubbed_printf("RM_OPA_CI");
            } else {
                if (w1 & 0x8) {
                    stubbed_printf("AA | ");
                }
                if (w1 & 0x10) {
                    stubbed_printf("Z_CMP | ");
                }
                if (w1 & 0x20) {
                    stubbed_printf("Z_UPD | ");
                }
                if (w1 & 0x40) {
                    stubbed_printf("IM_RD | ");
                }
                if (w1 & 0x80) {
                    stubbed_printf("CLR_ON_CVG | ");
                }
                switch ((w1 >> 8) & 3) {
                    case 0:
                        stubbed_printf("CVG_CLAMP | ");
                        break;
                    case 1:
                        stubbed_printf("CVG_WRAP | ");
                        break;
                    case 2:
                        stubbed_printf("CVG_FULL | ");
                        break;
                    case 3:
                        stubbed_printf("CVG_SAVE | ");
                        break;
                }
                switch ((w1 >> 10) & 3) {
                    case 0:
                        stubbed_printf("OPA | ");
                        break;
                    case 1:
                        stubbed_printf("INTER | ");
                        break;
                    case 2:
                        stubbed_printf("XLU | ");
                        break;
                    case 3:
                        stubbed_printf("DECAL | ");
                        break;
                }
                if (w1 & 0x1000) {
                    stubbed_printf("CVG_X_ALPHA | ");
                }
                if (w1 & 0x2000) {
                    stubbed_printf("ALPHA_CVG_SEL | ");
                }
                if (w1 & 0x4000) {
                    stubbed_printf("FORCE_BL | ");
                }
            }
            stubbed_printf(");\n");
            break;
        case 0x10:
            stubbed_printf("%20s: gSetBlender() - UNSUPPORTED\n");
            break;
    }
    return 8;
}

/* JFG: walks the geometry mode bits set in the command's second word (the prints were compiled out). */
s32 dl_cmd_print_geometry_mode(u32 *cmd, s32 arg1, UNUSED s32 arg2) {
    char buffer[0x50];
    UNUSED s32 pad;
    s32 flags;
    s32 bit;
    s32 first;

    bit = 1;
    first = TRUE;
    flags = cmd[1];
    sprintf(buffer, "%20s", arg1, flags);
    stubbed_printf("%20s: %s(");
    for (; bit <= 0x100000; bit <<= 1) {
        if (flags & bit) {
            if (first) {
                first = FALSE;
            } else {
                stubbed_printf("|");
            }
            switch (bit) {
                case 0x1:
                    stubbed_printf("G_ZBUFFER");
                    break;
                case 0x2:
                    stubbed_printf("G_TEXTURE_ENABLE");
                    break;
                case 0x4:
                    stubbed_printf("G_SHADE");
                    break;
                case 0x200:
                    stubbed_printf("G_SHADING_SMOOTH");
                    break;
                case 0x1000:
                    stubbed_printf("G_CULL_FRONT");
                    break;
                case 0x2000:
                    stubbed_printf("G_CULL_BACK");
                    break;
                case 0x10000:
                    stubbed_printf("G_FOG");
                    break;
                case 0x20000:
                    stubbed_printf("G_LIGHTING");
                    break;
                case 0x40000:
                    stubbed_printf("G_TEXTURE_GEN");
                    break;
                case 0x80000:
                    stubbed_printf("G_TEXTURE_GEN_LINEAR");
                    break;
                case 0x100000:
                    stubbed_printf("G_LOD");
                    break;
            }
        }
    }
    stubbed_printf(");\n");
    return 8;
}
