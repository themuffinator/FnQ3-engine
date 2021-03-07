#include "tr_local.h"
// tr_arc.c -- UI aspect ratio correction hacks

typedef enum {
	SCR_NONE,
	SCR_H_LEFT,
	SCR_H_MID,
	SCR_H_RIGHT,
	SCR_V_TOP,
	SCR_V_MID,
	SCR_V_BOTTOM
} scrAlign;

typedef enum {
	STRETCH_NONE,
	STRETCH_HORZ,
	STRETCH_VERT,
	STRETCH_ALL
} scrAllowStretch;

static qboolean	scr_init = qfalse;

typedef struct {
	int			x1, y1, x2, y2;
	int			width_min, width_max;
	int			height_min, height_max;
	int			screen_horz, screen_vert;
	int			allow_stretch;

	int			cvar_mod_count;
	qboolean	valid;
} scrRegion_t;

typedef struct {
	scrRegion_t reg[SCR_MAX_REGIONS];
} scrRegions_t;

static scrRegions_t sr;

int RE_CharToScreen( const qboolean vertical, const char *in ) {
	if ( in ) {
		if ( vertical ) {
			if ( in[0] == 't' || in[0] == 'T' )
				return SCR_V_TOP;
			if ( in[0] == 'm' || in[0] == 'M' )
				return SCR_V_MID;
			if ( in[0] == 'b' || in[0] == 'B' )
				return SCR_V_BOTTOM;
		} else {
			if ( in[0] == 'l' || in[0] == 'L' )
				return SCR_H_LEFT;
			if ( in[0] == 'c' || in[0] == 'C' )
				return SCR_H_MID;
			if ( in[0] == 'r' || in[0] == 'R' )
				return SCR_H_RIGHT;
		}
	}
	return SCR_NONE;
}

int RE_CharToStretch( const char *in ) {
	if ( in ) {
		if ( in[0] == 'v' || in[0] == 'V' )
			return STRETCH_VERT;
		if ( in[0] == 'h' || in[0] == 'H' )
			return STRETCH_HORZ;
		if ( in[0] == 'a' || in[0] == 'A' )
			return STRETCH_ALL;
	}
	return STRETCH_NONE;
}

void RE_UpdateScreenRegion( const int index ) {
	int i;

	if ( r_arc_hud->integer < 2 ) return;

	if ( !scr_init ) {
		memset( &sr, 0, sizeof( sr ) );
	}

	for ( i = 0; i < SCR_MAX_REGIONS; i++ ) {
		if ( scr_init && index >= 0 && i != index ) continue;
		if ( !scr_init || sr.reg[i].cvar_mod_count != r_arc_region[i]->modificationCount ) {
			char *s[11];
			char v[1024];

			sr.reg[i].cvar_mod_count = r_arc_region[i]->modificationCount;
			if ( !r_arc_region[i]->string ) continue;

			strcpy( v, r_arc_region[i]->string );
			Com_Split( v, s, 11, ' ' );

			sr.reg[i].x1 = Q_atof(s[0]);
			sr.reg[i].y1 = Q_atof(s[1]);
			sr.reg[i].x2 = Q_atof(s[2]);
			sr.reg[i].y2 = Q_atof(s[3]);
			sr.reg[i].width_min = Q_atof(s[4]);
			sr.reg[i].width_max = Q_atof(s[5]);
			sr.reg[i].height_min = Q_atof(s[6]);
			sr.reg[i].height_max = Q_atof(s[7]);
			sr.reg[i].screen_horz = RE_CharToScreen( qfalse, s[8] );
			sr.reg[i].screen_vert = RE_CharToScreen( qtrue, s[9] );
			sr.reg[i].allow_stretch = RE_CharToStretch( s[10] );
#if 0
			ri.Printf( PRINT_ALL, S_COLOR_MAGENTA "region %i updated: x1=%i y1=%i x2=%i y2=%i wmin=%i wmax=%i hmin=%i hmax=%i shorz=%i svert=%i strch=%i\n",
				i+1, sr.reg[i].x1, sr.reg[i].y1, sr.reg[i].x2, sr.reg[i].y2,
				sr.reg[i].width_min, sr.reg[i].width_max, sr.reg[i].height_min, sr.reg[i].height_max,
				sr.reg[i].screen_horz, sr.reg[i].screen_vert, sr.reg[i].allow_stretch );
#endif
			if ( sr.reg[i].x1 && sr.reg[i].x2 > sr.reg[i].x1 && sr.reg[i].y1 && sr.reg[i].y2 > sr.reg[i].y1 ) {
				sr.reg[i].valid = qtrue;
			} else {
				sr.reg[i].valid = qfalse;
			}

		}
	}

	if ( !scr_init ) scr_init = qtrue;
}


/*
=============
RE_ScrCoordToAlign

Check coordinates to check if left/right/top/bottom screen space should be applied. Uses 640x480.
=============
*/
static int RE_ScrCoordToAlign( const qboolean vertical, const int x1, const int y1, const int x2, const int y2, int *allow_stretch ) {
	int i, w, h;

	w = x2 - x1;
	h = y2 - y1;

	for ( i = 0; i < SCR_MAX_REGIONS; i++ ) {
		if ( !sr.reg[i].valid ) continue;

		if ( x1 < sr.reg[i].x1 ) continue;
		if ( x2 > sr.reg[i].x2 ) continue;
		if ( y1 < sr.reg[i].y1 ) continue;
		if ( y2 > sr.reg[i].y2 ) continue;

		if ( sr.reg[i].width_max && w > sr.reg[i].width_max ) continue;
		if ( sr.reg[i].width_min && w < sr.reg[i].width_min ) continue;
		if ( sr.reg[i].height_max && h > sr.reg[i].height_max ) continue;
		if ( sr.reg[i].height_min && h < sr.reg[i].height_min ) continue;

		*allow_stretch = sr.reg[i].allow_stretch;

		if ( vertical ) {
			return sr.reg[i].screen_vert ? sr.reg[i].screen_vert : SCR_NONE;
		} else {
#if 0
			if ( sr.reg[i].allow_stretch )
			ri.Printf( PRINT_ALL, "Adjusting an element horizontally to region %i: x1=%i y1=%i x2=%i y2=%i (rx1=%i ry1=%i rx2=%i ry2=%i) screen=%i strch=%i\n",
				i,
				x1, y1, x2, y2,
				sr.reg[i].x1, sr.reg[i].x2, sr.reg[i].y1, sr.reg[i].y2,
				sr.reg[i].screen_horz ? sr.reg[i].screen_horz : SCR_NONE,
				sr.reg[i].allow_stretch
			);
#endif
			return sr.reg[i].screen_horz ? sr.reg[i].screen_horz : SCR_NONE;
		}
	}
	return vertical ? SCR_V_MID : SCR_H_MID;
}


/*
=============
RE_ScaleCorrection

Correct scaling and positioning for elements
=============
*/
void RE_ScaleCorrection( float *x, float *y, float *w, float *h, const int forceMode ) {
	const int mode = ( forceMode >= 0 ) ? forceMode : r_arc_hud->integer;
	int allow_stretch;

	if ( !mode || backEnd.isHyperspace || ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) ) return;
	else {
		float xScale, yScale;
		float mx, my, mw, mh;

		mx = (float)*x;
		my = (float)*y;
		mw = (float)*w;
		mh = (float)*h;

		xScale = ( (float)glConfig.vidWidth / 640.0 );
		yScale = ( (float)glConfig.vidHeight / 480.0 );

		//if x > 640 it's safe to assume the mod already has widescreen adjustments, so disable this
		if ( ( mx / xScale ) > 640.0f ) {
			ri.Cvar_Set( "r_arc_hud", "0" );
			return;
		}

		// no adjustments needed for 4:3
		if ( xScale == yScale ) return;

		switch ( mode ) {
		case 1:	// uniform fit to screen
			{
				if ( xScale > yScale ) {	// wide aspect - scale x w to height
					if ( mx ) {
						mx /= xScale;
						mx *= yScale;
						mx += ( glConfig.vidWidth - ( 640.0 * yScale ) ) * 0.5;
					}
					if ( mw ) {
						mw /= xScale;
						mw *= yScale;
					}
				} else {	// narrow/tall aspect - scale h to width
					if ( my ) {
						my /= yScale;
						my *= xScale;
						my += ( glConfig.vidHeight - ( 480.0 * xScale ) ) * 0.5;
					}
					if ( mh ) {
						mh /= yScale;
						mh *= xScale;
					}
				}
				break;
			}
		case 2:	// screen adjusted uniform scale
			{
				if ( xScale > yScale ) {	// wide aspect
					int sAlign;
					
					// revert to 640x480
					if ( mx ) mx /= xScale;
					if ( mw ) mw /= xScale;
					if ( my ) my /= yScale;
					if ( mh ) mh /= yScale;

					// determine horizontal screen space alignment
					sAlign = RE_ScrCoordToAlign( qfalse, mx, my, mx + mw, my + mh, &allow_stretch );
					//if ( sAlign != SCR_V_MID && sAlign != SCR_H_MID ) ri.Printf( PRINT_ALL, S_COLOR_MAGENTA "adjusted an element to region %i: x1=%f y1=%f x2=%f y2=%f\n", sAlign, mx, my, (int)(mx + mw), (int)(my + mh) );
					//if ( allow_stretch ) ri.Printf( PRINT_ALL, "streeeeeeetch = %i\n", allow_stretch );

					if ( allow_stretch != STRETCH_HORZ && allow_stretch != STRETCH_ALL )
						xScale = yScale;

					// scale back up uniformly
					mx *= xScale;
					mw *= xScale;
					my *= yScale;
					mh *= yScale;

					// adjust horizontal position
					switch ( sAlign ) {
					case SCR_H_MID:
						mx += ( glConfig.vidWidth - ( 640.0f * yScale ) ) * 0.5f;
						break;
					case SCR_H_RIGHT:
						mx += glConfig.vidWidth - ( 640.0f * yScale );
						break;
					default: //SCR_H_LEFT
						break;
					}
				} else {	//narrow/tall aspect
					int sAlign;

					// revert to 640x480
					if ( mx ) mx /= xScale;
					if ( mw ) mw /= xScale;
					if ( my ) my /= yScale;
					if ( mh ) mh /= yScale;

					// determine horizontal screen space alignment
					sAlign = RE_ScrCoordToAlign( qtrue, mx, my, mx + mw, my + mh, &allow_stretch );

					if ( allow_stretch != STRETCH_VERT && allow_stretch != STRETCH_ALL )
						yScale = xScale;

					// scale back up uniformly
					if ( mx ) mx *= xScale;
					if ( mw ) mw *= xScale;
					if ( my ) my *= yScale;
					if ( mh ) mh *= yScale;

					// adjust horizontal position
					switch ( sAlign ) {
					case SCR_V_MID:
						my += ( glConfig.vidHeight - ( 480.0f * xScale ) ) * 0.5f;
						break;
					case SCR_V_BOTTOM:
						my += glConfig.vidHeight - ( 480.0f * xScale );
						break;
					default: //SCR_V_TOP
						break;
					}
				}
				break;
			}

		default: break;
		}

		*x = mx;
		*y = my;
		*w = mw;
		*h = mh;
	}
}


/*
=============
RE_StretchAspectPic
=============
*/
void RE_StretchAspectPic( float x, float y, float w, float h,
	float s1, float t1, float s2, float t2, qhandle_t hShader, const qboolean cgame ) {
	stretchPicCommand_t *cmd;
	const char *mod = ri.Cvar_VariableString( "fs_game" );

	if ( !tr.registered ) return;
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) return;
	cmd->commandId = RC_STRETCH_PIC;

	cmd->shader = R_GetShaderByHandle( hShader );

	if ( backEnd.isHyperspace || ( backEnd.refdef.rdflags & RDF_HYPERSPACE ) ) {
		goto shader_parms;
	}

	if ( !cgame && !strstr( cmd->shader->name, "ui/assets/" ) ) {
		if ( !Q_stricmp( mod, "threewave" ) && r_arc_threewave_menu_fix->integer ) {
			float xScale = glConfig.vidWidth / 640.0;
			float yScale = glConfig.vidHeight / 480.0;

			switch ( r_arc_threewave_menu_fix->integer ) {
			case 1:
				x -= ( (float)glConfig.vidWidth - ( ( 640.0f ) * ( (float)glConfig.vidHeight / 480.0f ) ) ) / 2;
				break;
			default:
				w /= xScale;
				w *= yScale;
				x /= xScale;
				x *= yScale;
				break;
			}
		}
		goto shader_parms;
	}
	// detect crosshairs and fix aspect
	else if ( strstr( cmd->shader->name, "crosshair" ) && r_arc_crosshairs->integer ) {
		float oldW = w;

		// excessive dawn already adjusts crosshairs for widescreen
		if ( Q_stricmp( mod, "edawn" ) && Q_stricmp( mod, "excessiveplus" ) ) {
			w /= ( glConfig.vidWidth / 640.0 );
			w *= ( glConfig.vidHeight / 480.0 );

			x += ( oldW - w ) / 2;
		}
	}

	// detect elements and fix aspect
	// uniform fit
	else {
		RE_ScaleCorrection( &x, &y, &w, &h, -1 );
	}

shader_parms:
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->s1 = s1;
	cmd->t1 = t1;
	cmd->s2 = s2;
	cmd->t2 = t2;
}
