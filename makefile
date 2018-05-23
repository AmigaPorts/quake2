PRODUCT = quake2
INSTLOCATION =
SRC = amiga/cd_amiga.c client/cl_cin.c client/cl_ents.c client/cl_fx.c client/cl_input.c \
	  client/cl_inv.c client/cl_main.c client/cl_newfx.c client/cl_parse.c client/cl_scrn.c \
	  client/cl_pred.c client/cl_tent.c client/cl_view.c client/console.c\
	  qcommon/cmd.c qcommon/cmodel.c qcommon/common.c qcommon/crc.c qcommon/cvar.c qcommon/files.c\
	  amiga/in_amiga.c client/keys.c game/m_flash.c qcommon/md4.c client/menu.c qcommon/net_chan.c \
	  amiga/net_amiga.c qcommon/pmove.c game/q_shared.c \
	  amiga/q_shamiga.c client/qmenu.c \
	  client/snd_dma.c client/snd_mem.c client/snd_mix.c \
	  amiga/snd_amiga.c server/sv_ccmds.c server/sv_ents.c server/sv_game.c \
	  server/sv_init.c server/sv_main.c server/sv_send.c server/sv_user.c \
	  server/sv_world.c amiga/sys_amiga.c amiga/in_mouse.c amiga/vid_dll.c amiga/vid_menu.c snd_ahi/snd_ahi.c \
	  amiga/qgl_amiga.c	amiga/gla_imp.c amiga/renderercache.c #\
	  #amiga/in_joy.c amiga/in_psx.c amiga/in_pcjoy.c amiga/twfsound_cd.c

GAME_SRC = game/g_ai.c game/g_chase.c game/g_cmds.c game/g_combat.c game/g_func.c \
	game/g_items.c game/g_main.c game/g_misc.c \
	  game/g_monster.c game/g_phys.c game/g_save.c game/g_spawn.c game/g_svcmds.c game/g_target.c game/g_trigger.c \
	  game/g_turret.c game/g_utils.c game/g_weapon.c game/m_actor.c game/m_berserk.c game/m_boss2.c game/m_boss3.c \
	  game/m_boss31.c game/m_boss32.c game/m_brain.c game/m_chick.c game/m_flipper.c game/m_float.c \
	  game/m_flyer.c game/m_gladiator.c game/m_gunner.c game/m_hover.c game/m_infantry.c game/m_insane.c game/m_medic.c \
	  game/m_move.c game/m_mutant.c game/m_parasite.c game/m_soldier.c game/m_supertank.c game/m_tank.c game/p_client.c \
	  game/p_hud.c game/p_trail.c game/p_view.c game/p_weapon.c #game/q_shared.c

REF_SRC = ref_gl/gl_image.c ref_gl/gl_draw.c ref_gl/gl_light.c \
	ref_gl/gl_mesh.c ref_gl/gl_model.c ref_gl/gl_rmain.c ref_gl/gl_rmisc.c ref_gl/gl_rsurf.c ref_gl/gl_warp.c

SRC += $(GAME_SRC)
SRC += $(REF_SRC)

OBJ = $(SRC:.c=.o)

#GAME_OBJ = $(GAME_SRC:.c=.o)

include amiga/config.mk

