#include "client.h"
#include "in.h"

#include <intuition/intuition.h>

cvar_t      *m_filter;
cvar_t      *in_mouse;

qboolean    mlooking;

int         mouse_x, mouse_y;
int         old_mouse_x, old_mouse_y;

qboolean    mouseactive;
qboolean    mouseinitialized;


void IN_MLookDown (void)
{
    mlooking = true;
}

void IN_MLookUp (void)
{
    mlooking = false;
    if (!freelook->value && lookspring->value)
	IN_CenterView ();
}
/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (void)
{
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void)
{
}

/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
    cvar_t *cv;

    mouseinitialized=FALSE;
    mouseactive=FALSE;

//    if ( COM_CheckParm ("-nomouse") )
//        return;

    cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);
    if ( cv && !cv->value )
	return;

    Com_Printf("Mouse initialized\n");
    mouseinitialized = TRUE;
    mouseactive = TRUE;
}

/*
===========
IN_MouseEvent
===========
*/

void IN_MouseEvent (int mstate)
{
}

static int MouseX;
static int MouseY;

void IN_GetMouseMove(struct IntuiMessage *msg)
{
	MouseX=msg->MouseX;
	MouseY=msg->MouseY;
}

/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int             mx, my;

	if (!mouseactive)
		return;

	mx = (float)MouseX;
	my = (float)MouseY;
	MouseX = 0;
	MouseY = 0;

       
#if 0
	if (!mx && !my)
		return;
#endif

	if (m_filter->value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}

	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x *= (sensitivity->value*10.0);
	mouse_y *= (sensitivity->value*10.0);

	// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe->value && mlooking ))
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	if ( (mlooking || freelook->value) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
	}
	else
	{
		cmd->forwardmove -= m_forward->value * mouse_y;
	}

}

void IN_InitMouse()
{
    // mouse variables
    m_filter                = Cvar_Get ("m_filter",                 "0",        0);
    in_mouse                = Cvar_Get ("in_mouse",                 "1",        CVAR_ARCHIVE);

    // Mouse commands
    Cmd_AddCommand ("+mlook", IN_MLookDown);
    Cmd_AddCommand ("-mlook", IN_MLookUp);

    // Start the mouse
    if (mouseinitialized == false)
	IN_StartupMouse();
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
}

void IN_ShutdownMouse(void)
{
}
