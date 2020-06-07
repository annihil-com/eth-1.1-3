// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include <unistd.h>

#include "eth.h"

/*
==============================
Some utils functions
==============================
*/

int getSpawntimer(qboolean enemySpawn) {
	int team = eth.clientInfo[eth.cg_snap->ps.clientNum].team;
	int limbotime = 0;

	// Reverse the value from clientinfo[myself].team to make the function return the spawn time of the other team
    if(enemySpawn == qtrue) {
		if (team == TEAM_AXIS) {
			team = TEAM_ALLIES;
			limbotime = eth.cg_bluelimbotime;
		} else if (team == TEAM_ALLIES) {
			team = TEAM_AXIS;
			limbotime = eth.cg_redlimbotime;
		} else
			return -1;
    } else {
		if (team == TEAM_AXIS) {
			limbotime = eth.cg_redlimbotime;
		} else if (team == TEAM_ALLIES) {
			limbotime = eth.cg_bluelimbotime;
		} else
			return -1;
    }
    
    // Sanity check
    if (limbotime == 0) {
    	ethLog("warning: can't get spawntimer for team %i", team);
    	return -1;
    }

    return (int)(1 + (limbotime - ((eth.cgs_aReinfOffset[team] + eth.cg_time - eth.cgs_levelStartTime) % limbotime)) * 0.001f);
}

int findSatchel() {
	int entityNum = 0;
	for (; entityNum < MAX_GENTITIES; entityNum++) {
		if ((eth.cg_entities[entityNum].currentState->weapon == WP_SATCHEL)
				&& (eth.cg_entities[entityNum].currentState->clientNum == eth.cg_snap->ps.clientNum)
				&& (!VectorCompare(eth.entities[entityNum].origin, vec3_origin)))
			return entityNum;
	}
	#ifdef ETH_DEBUG
		ethDebug("satchel cam: don't find satchel for %i", eth.cg_snap->ps.clientNum);
	#endif
	return -1;
}

qboolean isKeyActionDown(char *action) {
	int key1, key2;
	orig_syscall(CG_KEY_BINDINGTOKEYS, action, &key1, &key2);

	if (syscall_CG_Key_IsDown(key1) || syscall_CG_Key_IsDown(key2))
		return qtrue;
	else
		return qfalse;
}

int getIdByName (const char *name, int len) {
	int i;
	if (!len)
		len = strlen(name);
	
	for (i=0; i < MAX_CLIENTS; i++)
		if (eth.clientInfo[i].infoValid && !strncmp(eth.clientInfo[i].name, name, len))
			return i;
	return -1;
}

qboolean isVisible(vec3_t target) {
	trace_t trace;
	eth.CG_Trace(&trace, eth.cg_refdef.vieworg, NULL, NULL, target, eth.cg_snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_CORPSE);
	return (trace.fraction == 1.0f);
}

qboolean isPlayerVisible(vec3_t target, int player) {
	trace_t traceVisible;
	eth.CG_Trace(&traceVisible, eth.cg_refdef.vieworg, NULL, NULL, target, eth.cg_snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_CORPSE);

	trace_t tracePlayer;
	eth.CG_Trace(&tracePlayer, eth.cg_refdef.vieworg, NULL, NULL, target, eth.cg_snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);

	//printf("XXX: want %i got %i\n", player, tracePlayer.entityNum);

	return ((traceVisible.fraction == 1.0f) && (tracePlayer.entityNum == player));
}

void ethLog(const char *format, ...) {
	printf("eth: ");

	va_list arglist;
	va_start(arglist, format);
		vprintf(format, arglist);
	va_end(arglist);

	printf("\n");

	#ifdef ETH_DEBUG
		// If log file
		if (debugFile) {
			fprintf(debugFile, "log: ");
			va_list arglist;
			va_start(arglist, format);
				vfprintf(debugFile, format, arglist);
			va_end(arglist);	
			fprintf(debugFile, "\n");
		}
	#endif
}

#ifdef ETH_DEBUG

FILE *debugFile = NULL;
void ethDebug(const char *format, ...) {
	// If log file
	if (debugFile) {
		va_list arglist;
		va_start(arglist, format);
			vfprintf(debugFile, format, arglist);
		va_end(arglist);	
		fprintf(debugFile, "\n");
	}
}

#endif // ETH_DEBUG

void fatalError(const char *msg) {
	char str[256];
	snprintf(str, sizeof(str), "A fatal error has occured. You must restart the game.\n\nError: %s\n\n\n*nixCoders eth-v%s", msg, ETH_VERSION);
	orig_syscall(CG_ERROR, str);
	#ifdef ETH_DEBUG
		ethLog("fatal error: %s", msg);
	#endif
}

// Helper function for get ouput of a system command
// This function return the first line of this command output
char *getOutputSystemCommand(const char *command) {
	static char buf[256];
	memset(buf, 0, sizeof(buf));

	FILE *cmd = popen(command, "r");

	int c; 
	int count = 0;
	while (((c = getc(cmd)) != EOF) && (count < (sizeof(buf) - 1))) {
		if (c == '\n')
			break;
		buf[count++] = c;
	}

	pclose(cmd);
	return buf;
}

// If cgame is load, send a UI_PRINT message else just show a message at console,
// WARNING ! use this function only when call from ui vmMain. ex: irc and game command
#define MESSAGE_COLOR "^n"
void gameMessage(qboolean forceConsole, char *format, ...) {
	char msg[MAX_SAY_TEXT];
	memset(msg, 0, sizeof(msg));
	char buffer[sizeof(msg)];
	memset(buffer, 0, sizeof(buffer));
	va_list arglist;

	va_start(arglist, format);
		vsnprintf(msg, sizeof msg, format, arglist);
	va_end(arglist);

	if (eth.hookLoad && !forceConsole) {
		snprintf(buffer, sizeof(buffer), "echo \"" MESSAGE_COLOR "%s\"\n", msg);
		orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, buffer);
	} else {
		snprintf(buffer, sizeof(buffer), MESSAGE_COLOR "%s\n", msg);
		orig_syscall(UI_PRINT, buffer);
	}
}


void doAutoVote(char *str) {
	char *ptr = strstr(str, "^7 called a vote.  Voting for: ");
	int id = getIdByName(str, ptr - str);

	// id not found
	if (id == -1)
		return;

	// who votes
	switch (eth.clientInfo[id].targetType) {
		case PLIST_FRIEND:	
			syscall_CG_SendConsoleCommand("vote yes\n");
			return;
		case PLIST_TEAMKILLER:
			syscall_CG_SendConsoleCommand("vote no\n");
			return;
		default:
			break;
	}
	
	// Voter is not a friend/teamkiller
	ptr += strlen("^7 called a vote.  Voting for: ");

	if (!strncmp(ptr, "KICK ", 5) || !strncmp(ptr, "MUTE ", 5)) {
		ptr += 5;
		id = getIdByName(ptr, strrchr(ptr, '\n') - ptr);

		// id not found
		if (id == -1)
			return;

		// vote against client
		if (id == eth.cg_clientNum) {
			syscall_CG_SendConsoleCommand("vote no\n");
			return;
		}
		
		// who is voted against
		switch (eth.clientInfo[id].targetType) {
			case PLIST_FRIEND:
				syscall_CG_SendConsoleCommand("vote no\n");
				return;
			case PLIST_TEAMKILLER:
				syscall_CG_SendConsoleCommand("vote yes\n");
				return;
			default:
				break;
		}
	}
}

// Auto request medic
#define TIME_BETWEEN_REQUEST 5000
void autoRequestMedic() {
	if (eth.cg_snap->ps.clientNum != eth.cg_clientNum)
		return;

	static int lastRequestTime = 0;
	if (lastRequestTime + TIME_BETWEEN_REQUEST > eth.cg_time)
		return;

	// If need revive
	if (eth.cg_entities[eth.cg_clientNum].currentState->eFlags & EF_DEAD) {
		int vsay = (int)(2.0f * (rand() / (RAND_MAX + 1.0f)));
		if (!vsay)
			syscall_CG_SendConsoleCommand("vsay_team Medic\n");
		else
			syscall_CG_SendConsoleCommand("vsay_team FTReviveMe\n");
		
		lastRequestTime = eth.cg_time;
	// If need health
	} else if (eth.cg_snap->ps.stats[STAT_HEALTH] < 40) {
		int vsay = (int)(2.0f * (rand() / (RAND_MAX + 1.0f)));
		if (!vsay)
			syscall_CG_SendConsoleCommand("vsay_team Medic\n");
		else
			syscall_CG_SendConsoleCommand("vsay_team FTHealMe\n");
		
		lastRequestTime = eth.cg_time;
	}
}

// Calc muzzle
void setCurrentMuzzle() {
	vec3_t forward, right, up;

	VectorCopy(eth.cg_snap->ps.origin, eth.muzzle);
	eth.muzzle[2] += eth.cg_snap->ps.viewheight;
	AngleVectors(eth.cg_snap->ps.viewangles, forward, right, up);

	switch (eth.cg_snap->ps.weapon)	{
		case WP_PANZERFAUST:
			VectorMA(eth.muzzle, 10, right, eth.muzzle);
			break;
		case WP_DYNAMITE:
		case WP_GRENADE_PINEAPPLE:
		case WP_GRENADE_LAUNCHER:
		case WP_SATCHEL:
		case WP_SMOKE_BOMB:
			VectorMA(eth.muzzle, 20, right, eth.muzzle);
			break;
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
			VectorMA(eth.muzzle, -6, right, eth.muzzle);
			VectorMA(eth.muzzle, -4, up, eth.muzzle);
			break;
		default:
			VectorMA(eth.muzzle, 6, right, eth.muzzle);
			VectorMA(eth.muzzle, -4, up, eth.muzzle);
			break;
	}
	SnapVector(eth.muzzle);
}

/*
==============================
 Config stuff
==============================
*/

char *getConfigFilename() {
	// If config filename is set by enviromment var
	if (getenv("ETH_CONF_FILE"))
		return getenv("ETH_CONF_FILE");
	
	static char filename[PATH_MAX];
	snprintf(filename, sizeof(filename), "%s/%s", getenv("HOME"), ETH_CONFIG_FILE);
	return filename;
}

void readConfig() {
	FILE *file;

	// Init all user vars with the default value
	int count = 0;
	for (; count < VARS_TOTAL; count++)
		seth.value[count] = seth.vars[count].defaultValue;

	if ((file = fopen(getConfigFilename(), "rb")) == NULL)
		return;

	// Get config file line by line
	char line[32];
	while (fgets(line, sizeof(line) - 1, file) != 0) {
		char *sep = strrchr(line, '=');
		*sep = '\0';	// Separate name from value
		// Search this var
		int count = 0;
		for (; count < VARS_TOTAL; count++) {
			if (!seth.vars[count].cvarName) {
				ethLog("readConfig: error: VAR_%i undefine", count);
			} else if (!strcmp(line, seth.vars[count].cvarName)) {
				seth.value[count] = atof(sep + 1);
				break;
			} else if ((count + 1) == VARS_TOTAL) {
				ethLog("readConfig: don't know this var: [%s]", line);
			}
			if (count == VAR_CHUD)
				if (atoi(sep + 1))
					syscall_CG_SendConsoleCommand(va("cg_draw2d %s\n", seth.value[count] ? "0" : "1"));
		}
	}

	fclose(file);

}

void writeConfig() {
	FILE *file;
	
	if ((file = fopen(getConfigFilename(), "w")) == NULL) {
		ethLog("eth: can't write config file.");
		return;
	}
	
	int count = 0;
	for (; count < VARS_TOTAL; count++) {
		if (!seth.vars[count].cvarName)
			ethLog("writeConfig: error: VAR_%i undefine", count);
		else if (seth.value[count] == (float)(int)seth.value[count])
			fprintf(file, "%s=%i\n", seth.vars[count].cvarName, (int)seth.value[count]);
		else
			fprintf(file, "%s=%.2f\n", seth.vars[count].cvarName, seth.value[count]);
	}

	fclose(file);
}

/*
==============================
 Console actions
==============================
*/

void initActions() {
	eth.actions[ACTION_ATTACK]		= (ethAction_t){ 0, 0, "+attack\n",				"-attack\n" };
	eth.actions[ACTION_BACKWARD]	= (ethAction_t){ 0, 0, "+backward\n",			"-backward\n" };
	eth.actions[ACTION_BINDMOUSE1]	= (ethAction_t){ 1, 1, "bind mouse1 +attack\n",	"unbind mouse1\n" };
	eth.actions[ACTION_CROUCH]		= (ethAction_t){ 0, 0, "+movedown\n",			"-movedown\n" };
	eth.actions[ACTION_JUMP]		= (ethAction_t){ 0, 0, "+moveup\n",				"-moveup\n" };
	eth.actions[ACTION_PRONE]		= (ethAction_t){ 0, 0, "+prone; -prone\n",		"+moveup; -moveup\n" };
	eth.actions[ACTION_RUN]			= (ethAction_t){ 0, 0, "+sprint; +forward\n",	"-sprint; -forward\n" };
	eth.actions[ACTION_SCOREBOARD]	= (ethAction_t){ 0, 0, "cg_draw2d 1; +scores\n", "-scores; cg_draw2d 0\n" };
	eth.actions[ACTION_RELOAD]			= (ethAction_t){ 0, 0, "+reload\n",	"-reload\n" };
}

// Set an action if not already set.
void setAction(int action, int state) {
	if (state && !eth.actions[action].state)
		forceAction(action, state);
	else if (!state && eth.actions[action].state)
		forceAction(action, state);
}

// Use this with caution
void forceAction(int action, int state) {
	if (state) {
		eth.actions[action].state = 1;
		syscall_CG_SendConsoleCommand(eth.actions[action].startAction);
		#ifdef ETH_DEBUG
			ethDebug("forceAction: %s", eth.actions[action].startAction);
		#endif
	} else {
		eth.actions[action].state = 0;
		syscall_CG_SendConsoleCommand(eth.actions[action].stopAction);
		#ifdef ETH_DEBUG
			ethDebug("forceAction: %s", eth.actions[action].stopAction);
		#endif
	}
}

void resetAllActions() {
	int action;
	for (action=0; action<ACTIONS_TOTAL; action++)
		if (action != ACTION_SCOREBOARD)
			forceAction(action, eth.actions[action].defaultState);
}

/*
==============================
 Killing spree/stats/sound stuff
==============================
*/

// Spree sound
void playSpreeSound() {
	if ((eth.cg_time - eth.lastKillTime) < (SPREE_TIME * 1000)) {
		// Define spree levels range - TODO: Dirty. find a better way to play with level range
		typedef struct { int start; int end; } spreeLevel_t;
		#define SPREE_LEVEL_SIZE 4
		spreeLevel_t spreeLevels[SPREE_LEVEL_SIZE] = {
			{ SOUND_DOUBLEKILL1,	SOUND_MULTIKILL1 }	,	// Spree level 1
			{ SOUND_ULTRAKILL1,		SOUND_MONSTERKILL1 },
			{ SOUND_DOMINATING1,	SOUND_HOLYSHIT },
			{ SOUND_MONSTERKILL1,	SOUND_WICKEDSICK }	// Spree level 4
		};
		int spreeLevelMax = SPREE_LEVEL_SIZE - 1;

		// Modify level+sound values to fit to spreeLevels_t and eth.sounds order
		int level = eth.killSpreeCount - 2; // never '< 0' because first time here is with 2 kills
		if (level > spreeLevelMax)
			level = spreeLevelMax;
		int levelSize = spreeLevels[level].end - spreeLevels[level].start;
		int sound = (int)((float)(levelSize + 1) * rand() / (RAND_MAX + 1.0f));
		sound += spreeLevels[level].start;
		eth.spreelevel = sound;
		eth.startFadeTime = eth.cg_time;
		eth.s_level = level;
		orig_syscall(CG_S_STARTLOCALSOUND, eth.sounds[sound], CHAN_LOCAL_SOUND, 230);
	}
}

/* replaces tokens with correct value and returns pointer parsed string
	[c] - current spree count
	[v] - victim name
	[n] - clean victim name
	[t] - total kills
	[m] - double/triple/multikill msg
	[x]	- Player XP
	[p] - Player name
	[M] - Mod name
	[h] - Player health
	[k] - Killer name
	[K] - Clean killer name
	[q] - amount of punkbuster screenshots requested
*/
char *Format(char *in){
	char out[256];
	memset(out, 0, sizeof(out));
	
	int a, b, len = strlen(in);
	for (a = 0, b = 0; a < len; a++) {
		if ((a <= len-2) && (in[a] == '[' && in[a+2] == ']')) {
			switch (in[a+1]) {
				case 'v':
					strncat(out, eth.VictimName, sizeof(out));
					break;
				case 'n':
					strncat(out, Q_CleanStr(eth.VictimName), sizeof(out));
					break;
				case 't':
					strncat(out, va("%i", eth.killCount), sizeof(out));
					break;
				case 'x':
					strncat(out, va("%i", eth.cg_snap->ps.stats[STAT_XP]), sizeof(out));
					break;
				case 'q':
					strncat(out, va("%i", eth.pbss), sizeof(out));
					break;					
				case 'p':
					strncat(out, eth.clientInfo[eth.cg_clientNum].name, sizeof(out));
					break;
				case 'M':
				{
					char gameMod[20];
					syscall_CG_Cvar_VariableStringBuffer("fs_game", gameMod, sizeof(gameMod));
					strncat(out, gameMod, sizeof(out));
					break;
				}
				case 'k':
					strncat(out, eth.KillerName, sizeof(out));
					break;
				case 'K':
				{
					char *c = strdup(eth.KillerName);
					strncat(out, Q_CleanStr(c) , sizeof(out));
					free(c);
					break;
				}
				case 'h':
					strncat(out, va("%i", eth.cg_snap->ps.stats[STAT_HEALTH]), sizeof(out));
					break;								
				case 'c':
					strncat(out, va("%i", eth.killCountNoDeath), sizeof(out));
					break;
					//multikill + rampage msg ...
					case '1': {
						char *multikillmsg ;
						if (eth.killSpreeCount == 2) {
							multikillmsg = " ^1*** DOUBLEKILL***^7";
						}
						else if (eth.killSpreeCount == 3) {
										
							multikillmsg = " ^1*** MULTIKILL***^7";
						}
						else if (eth.killSpreeCount == 4) {
							multikillmsg = " ^1*** ULTRAKILL***^7";
						}
						else if (eth.killSpreeCount > 4) {
							multikillmsg = " ^1*** MONSTERKILL***^7";
						}
						else
							break;
						if (eth.killSpreeCount >= 2)
							sprintf(out, "^2%s ^7was owned! %s ", eth.VictimName, multikillmsg);
						break;
					}
					case '2': {
						char *multikillmsg;
			
						if (eth.killSpreeCount == 2) {
										
							multikillmsg = "^1<!^7DOUBLEKILL^1!>";
										
						}
						else if (eth.killSpreeCount == 3) {
										
							multikillmsg = "^1<!^7MULTIKILL^1!>";
										
						}
						else if (eth.killSpreeCount == 4) {
										
							multikillmsg = "^1<!^7ULTRAKILL^1!>";
										
						}
						else if (eth.killSpreeCount > 4) {
										
							multikillmsg = "^1<!^7MONSTERKILL^1!>";
						}
						else
							break;
						if (eth.killSpreeCount >= 2)
							sprintf(out, "%s ", multikillmsg);
						break;
					}
					case '3': {
						char *multikillmsg;
									
						if (eth.killSpreeCount == 2) {
										
							multikillmsg = " ^7::: ^2Doublekill ^7:::";
										
						}
						else if (eth.killSpreeCount == 3) {
										
							multikillmsg = " ^7::: ^2Multikill ^7:::";
										
						}
						else if (eth.killSpreeCount == 4) {
										
							multikillmsg = " ^7::: ^2Ultrakill ^7:::";
										
						}
						else if (eth.killSpreeCount >= 5) {
										
							multikillmsg = " ^7::: ^2Monsterkill ^7:::";
						}
						else
							break;
						if (eth.killSpreeCount >= 2)
							sprintf(out, "%s ", multikillmsg);
						break;
					}
					case 'e': {
						char *msg;
						if (eth.killCountNoDeath == 10) {
							msg = "^7... is ^/tied for the lead! ^%(10 frags in a row)";
						}
						else if (eth.killCountNoDeath == 15) {
							msg =  "^7... has ^3taken the lead! ^%(15 frags in a row)";
									
						}
						else if (eth.killCountNoDeath == 20) {
							msg = "^7... is on a ^3KILLING SPREE!!! ^%(20 frags in a row)";
						}	
						else if (eth.killCountNoDeath == 25) {
							msg = "^7... is ^/DOMINATING!!! ^%(25 frags in a row)";
						}
						else if (eth.killCountNoDeath == 30) {
							msg = "^7... is on a ^/RAMPAGE!!! ^%(30 frags in a row)";
						}
						else
							break;
						if (eth.killCountNoDeath >= 2)
							sprintf(out, "%s ", msg);
						break;
					}
					case '6': {
						char *killmsg;
						int randmsg = rand() % 19;
						if (eth.killCountNoDeath) {
							if (!randmsg)
								killmsg = strncpy(ksFormat, "Hey bug eyes!",sizeof(ksFormat));
						
							else if (randmsg == 1)
								killmsg = strncpy(ksFormat, "Hey bug eyes!",sizeof(ksFormat));
							else if (randmsg == 2)
								killmsg = strncpy(ksFormat, "Damn, I'm good!",sizeof(ksFormat));
							else if (randmsg == 3)
								killmsg = strncpy(ksFormat, "Now you see me, now you're dead!",sizeof(ksFormat));
							else if (randmsg == 4)
								killmsg = strncpy(ksFormat, "I like big guns and I not lie!",sizeof(ksFormat));
							else if (randmsg == 5)
								killmsg = strncpy(ksFormat, "Life is like a box of ammo!",sizeof(ksFormat));
							else if (randmsg == 6)
								killmsg = strncpy(ksFormat, "Go ahead, make my day!",sizeof(ksFormat));
							else if (randmsg == 7)
								killmsg = strncpy(ksFormat, "Stop eyeballin' me!",sizeof(ksFormat));
							else if (randmsg == 8)
								killmsg = strncpy(ksFormat, "See hello to my little friend!",sizeof(ksFormat));
							else if (randmsg == 9)
								killmsg = strncpy(ksFormat, "You guys suck!",sizeof(ksFormat));
							else if (randmsg == 10)
								killmsg = strncpy(ksFormat, "Oehhh, that's gonna leave a mark!",sizeof(ksFormat));
							else if (randmsg == 11)
								killmsg = strncpy(ksFormat, "Oehhh, that's gotta hurt!",sizeof(ksFormat));
							else if (randmsg == 12)
								killmsg = strncpy(ksFormat, "Jippy ka yay motherfucker!",sizeof(ksFormat));
							else if (randmsg == 13)
								killmsg = strncpy(ksFormat, "Life's a bitch and then YOU die!",sizeof(ksFormat));
							else if (randmsg == 14)
								killmsg = strncpy(ksFormat, "It's cluberin' time!",sizeof(ksFormat));
							else if (randmsg == 15)
								killmsg = strncpy(ksFormat, "I see dead people!",sizeof(ksFormat));
							else if (randmsg == 16)
								killmsg = strncpy(ksFormat, "I'm gonna kick your ass bitch!",sizeof(ksFormat));
							else if (randmsg == 17)
								killmsg = strncpy(ksFormat, "Die you son of a bitch!",sizeof(ksFormat));
							else if (randmsg == 18)
								killmsg = strncpy(ksFormat, "You're beautiful when you're dieing!",sizeof(ksFormat));
							else if (randmsg == 19)
								killmsg = strncpy(ksFormat, "Take that you dirty rat!",sizeof(ksFormat));
									
							else
								killmsg = strncpy(ksFormat, "Take that you dirty rat! ",sizeof(ksFormat));
						}
						else
							break;
						strncat(out, killmsg, sizeof(out));
						break;
					}
					case 'R': {
						char *msg;
						if (eth.killCountNoDeath == 10) {
							msg = "^7is on a  ^1KILLING SPREE!!! ^3(10 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 15) {
							msg = "^7is ^1DOMINATING!!! ^3(15 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 20) {
							msg = "^7is on a ^1RAMPAGE!!! ^3(20 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 25) {
							msg = "^7is ^1UNSTOPPABLE!!! ^3(25 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 30) {
							msg = "^7is ^1WICKED SICK!!! ^3(30 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 35) {
							msg = "^7is on a ^1MEGA KILLINGSPREE!!! ^3(35 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 40) {
							msg = "^7is ^1OMFGHAXOR!!! ^3(40 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 45) {
							msg = "^3I'M HARD AS FUCKING STEEL!!! ^3(45 frags in a row)";
						}
						else if  (eth.killCountNoDeath == 50) {
							msg = "^NOBODY CAN STOP ME NOW!!! ^3(50 frags in a row)";
						}
						else
							break;
						if (eth.killCountNoDeath >= 2)
							sprintf(out, "%s ", msg);	
						break;
					}
					case '5': {
						char *multikillmsg;
						int randmsg = rand() % 3;
						if (eth.killSpreeCount == 2) {
							if (!randmsg)
								multikillmsg = " ^1-> ^7doubleLEAK!";
							else if (randmsg == 1)
								multikillmsg = " ^1-> ^7doubleLEAK!";
							else
								multikillmsg = " ^1-> ^7doubleLEAK!";
						}
						else if (eth.killSpreeCount == 3) {
							if (!randmsg)
								multikillmsg = " ^1-> ^7multiLEAK!";
							else if (randmsg == 1)
								multikillmsg = " ^1-> ^7multiLEAK!"; 
							else
								multikillmsg = " ^1-> ^7multiLEAK!"; 
						}
						else if (eth.killSpreeCount == 4) {
							if (!randmsg)
								multikillmsg = " ^1-> ^7ultraLEAK!";
							else if (randmsg == 1)
								multikillmsg = " ^1-> ^7ultraLEAK!";
							else
								multikillmsg = " ^1-> ^7ultraLEAK!";
						}
						else if (eth.killSpreeCount > 4) {
							if (!randmsg)
								multikillmsg = " ^1-> ^7monsterLEAK!";
							else if (randmsg == 1)
								multikillmsg = " ^1-> ^7Holy Shit!!!!!LEAK";
							else
								multikillmsg = " ^1-> ^7monsterLEAK!";
						}
						else
							break;
						strncat(out, multikillmsg, sizeof(out));
						break;
					}
					case 'X': {
						char *killmsg;
						int randmsg = rand() % 24;
						if (eth.killCount) {
							if (!randmsg)
								killmsg = " ^7 + My Gun = +1 on my owned list.";
							else if (randmsg == 1)
								killmsg =  "^7... What? There's only one of you?";
							else if (randmsg == 2)
								killmsg = " ^7 You're a joke!";
							else if (randmsg == 3)
								killmsg =  "^7... suck it down!";
							else if (randmsg == 4)
								killmsg =  " ^7, go home!";
							else if (randmsg == 5)
								killmsg = " ^7... Suprise, asshole!";
							else if (randmsg == 6)
								killmsg = "^7, shit happens.";
							else if (randmsg == 7)
								killmsg = "^7, you are the weakest link, good-bye!";
							else if (randmsg == 8)
								killmsg = "^7- Mmmm, bad case of lead poisoning!";
							else if (randmsg == 9)
								killmsg =  " ^7is now in a better place!";
							else if (randmsg == 10)
								killmsg = " ^7 wanted to take part in my killingspree.";
							else if (randmsg == 11)
								killmsg = " ^7 enjoyed a ^3crisp ^7bullet to the head!";
							else if (randmsg == 12)
								killmsg = " ^7 is no more.";
							else if (randmsg == 13)
								killmsg = "^7, It's time to kick ass, and chew bubblegum... and i'm all outta gum.";
							else if (randmsg == 14)
								killmsg = "^7, it's time to abort your whole friggin' species.";
							else if (randmsg == 15)
								killmsg = "^7, Come get some.";
							else if (randmsg == 16)
								killmsg = " ^7... Get that crap outta here.";
							else if (randmsg == 17)
								killmsg = " ^7 got on the bad side of me... and ended up ^3DEAD^7!";
							else if (randmsg == 18)
								killmsg = "^7's brain made a mess on the floor... * clean up on isle";
							else if (randmsg == 19)
								killmsg = " ^7got in my way.";
							else if (randmsg == 20)
								killmsg = "^7... Eat shit and Die!";
							else if (randmsg == 21)
								killmsg = "^7, you suck.";
							else if (randmsg == 22)
								killmsg = "^7... Hehehe, what a mess.";
							else if (randmsg == 23)
								killmsg = "^7 was killed!";
							else if (randmsg == 24)
								killmsg = "^7, Looks like it's Time to Kill.";
							else
								killmsg = "^7 Time to Die.";
						}
						else
							break;
						strncat(out,killmsg, sizeof(out));
						break;
					}
					case 'C': {
						char *killmsg;
						int randmsg = rand() % 9;
						if (eth.killCountNoDeath) {
							if (!randmsg)
								killmsg = strncpy(ksFormat, "^2Eliminated",sizeof(ksFormat));
						
							else if (randmsg == 1)
								killmsg = strncpy(ksFormat, "^2Time To Die:",sizeof(ksFormat));
							else if (randmsg == 2)
								killmsg = strncpy(ksFormat, "^2And Stay Dead:",sizeof(ksFormat));
							else if (randmsg == 3)
								killmsg = strncpy(ksFormat, "^3Killed:",sizeof(ksFormat));
							else if (randmsg == 4)
								killmsg = strncpy(ksFormat, "^2Yes You SUCKED:",sizeof(ksFormat));
							else if (randmsg == 5)
								killmsg = strncpy(ksFormat, "^2Rest In Peace:",sizeof(ksFormat));
							else if (randmsg == 6)
								killmsg = strncpy(ksFormat, "^2TAXI for:",sizeof(ksFormat));
							else if (randmsg == 7)
								killmsg = strncpy(ksFormat, "^2Bring It On!",sizeof(ksFormat));
							else if (randmsg == 9)
								killmsg = strncpy(ksFormat, "^2Humiliated:",sizeof(ksFormat));
									
							else
								killmsg = strncpy(ksFormat, "^2DESTROYED: ",sizeof(ksFormat));
						}
						else
							break;
						strncat(out, killmsg, sizeof(out));
						break;
					}
					case '8': {
						char *killmsg;
						int randmsg = rand() % 12;
						if (eth.killCountNoDeath) {
							if (!randmsg)
								killmsg = strncpy(ksFormat, "^gOnly my weapon understands me.",sizeof(ksFormat));
						
							else if (randmsg == 1)
								killmsg = strncpy(ksFormat, "^gGuns don't kill people, I do!",sizeof(ksFormat));
							else if (randmsg == 2)
								killmsg = strncpy(ksFormat, "^gThe gene pool is stagnant and I am the minister of chlorine.",sizeof(ksFormat));
							else if (randmsg == 3)
								killmsg = strncpy(ksFormat, "^gYou probably thought you weren't goin' to die today? Surprise!",sizeof(ksFormat));
							else if (randmsg == 4)
								killmsg = strncpy(ksFormat, "^gHey I'm just trying to exercise my second-amendment rights here ya fuckin' Communist!",sizeof(ksFormat));
							else if (randmsg == 5)
								killmsg = strncpy(ksFormat, "^gYou probably think I'm not a nice person...",sizeof(ksFormat));
							else if (randmsg == 6)
								killmsg = strncpy(ksFormat, "^gI am The Law!",sizeof(ksFormat));
							else if (randmsg == 7)
								killmsg = strncpy(ksFormat, "^gThat one's cause I have ammo left in my gun!",sizeof(ksFormat));
							else if (randmsg == 8)
								killmsg = strncpy(ksFormat, "^gSomeone stole my donuts, and now you're all gonna pay!",sizeof(ksFormat));
							else if (randmsg == 9)
								killmsg = strncpy(ksFormat, "^gShut the fuck up, fat man, this ain't none of your god-damned business!",sizeof(ksFormat));
							else if (randmsg == 10)
								killmsg = strncpy(ksFormat, "^gOh man, I just shot Marvin in the face.",sizeof(ksFormat));
							else if (randmsg == 11)
								killmsg = strncpy(ksFormat, "^gYou call that begging? You can beg better than that.",sizeof(ksFormat));
							else if (randmsg == 12)
								killmsg = strncpy(ksFormat, "^gYou shoot me in a dream, you better wake up and apologize.",sizeof(ksFormat));
							else
								killmsg = "^g Time to Die. ";
						}
						else
							break;
						strncat(out,killmsg, sizeof(out));
						break;
					}
					case '9': {
						char *killmsg;
						int randmsg = rand() % 16;
						if (eth.killCount) {
							if (!randmsg)
								killmsg = "^3 Let's ^3ROCK!!";
							else if (randmsg == 1)
								killmsg = "^3 Haha, Its a hole in Your head!";
							else if (randmsg == 2)
								killmsg = "^3 Eat my ass and God bless..";
							else if (randmsg == 3)
								killmsg = "^3 Was soo weak and glassy-eyed";
							else if (randmsg == 4)
								killmsg = "^3, So I guess you’re not a fan of ^3Eth";
							else if (randmsg == 5)
								killmsg = "^3 Give Me Your Soul";
							else if (randmsg == 6)
								killmsg = "^3 Once More, You Have to be the Grown-Up";
							else if (randmsg == 7)
								killmsg = "^3 Your Blood is soo Sweet, like ^1m^3a^1r^3m^1a^3l^1a^3d^1e";
							else if (randmsg == 8)
								killmsg = "^3 There is no God, no heaven, Just ^3ME !!";
							else if (randmsg == 9)
								killmsg = "^3 Join Me in Hell, Won’t You?";
							else if (randmsg == 10)
								killmsg = "^3Yes You ^3SUCKED!!";
							else if (randmsg == 11)
								killmsg = "^3 I Am Your One-Way Ticket to Hell";
							else if (randmsg == 12)
								killmsg = "^3..How much is your dead body worth?...";
							else if (randmsg == 13)
								killmsg = "^3 Dead man walking? Not sure :)";
							else if (randmsg == 14)
								killmsg = "^3 Death Is Waiting 4 YOU!";
							else if (randmsg == 15)
								killmsg = "^3 Welcome to my list of fucked Bodies";
							else if (randmsg == 16)
								killmsg = "^3 Oh Yes, here is your Destination..";
							else
								killmsg = "^3 Time to Die. ";
						}
						else
							break;
						strncat(out,killmsg, sizeof(out));
						break;
					}
					case 'W': {
						char *multikillmsg;
						if (eth.killSpreeCount == 2) {
							multikillmsg = "^1<--|DOUBLEKILL|";
						}
						else if (eth.killSpreeCount == 3) {
							multikillmsg = "^1<--|MULTIKILL|";
						}
						else if (eth.killSpreeCount == 4) {
							multikillmsg = "^1<--|ULTRAKILL|";
						}
						else if (eth.killSpreeCount == 5) {
							multikillmsg = "^1<--|MONSTERKILL|";
						}
						else if (eth.killSpreeCount > 5) {
							multikillmsg = "^1<--|HOLY-SHIT|";
						}
						else
							break;
						strncat(out, multikillmsg, sizeof(out));
						break;
					}
					case 'm': {
						char *multikillmsg;
						int randmsg = rand() % 3;
						if (eth.killSpreeCount == 1) {
							if (!randmsg)
								multikillmsg = "EATEN";
							else if (randmsg == 1)
								multikillmsg = "OWNED";
							else
								multikillmsg = "ADIOS AMIGO";
						}
						if (eth.killSpreeCount == 2) {
							if (!randmsg)
								multikillmsg = "DOUBLE KILL";
							else if (randmsg == 1)
								multikillmsg = "PERFECT";
							else
								multikillmsg = "OWNED";
						}
						else if (eth.killSpreeCount == 3) {
							if (!randmsg)
								multikillmsg = "TRIPLE KILL";
							else if (randmsg == 1)
								multikillmsg = "GODLIKE";
							else
								multikillmsg = "LUDICROUS";
						}
						else if (eth.killSpreeCount == 4) {
							if (!randmsg)
								multikillmsg = "DOMINATING";
							else if (randmsg == 1)
								multikillmsg = "ULTRA KILL";
							else
								multikillmsg = "MEGA KILL";
						}
						else if (eth.killSpreeCount > 4) {
							if (!randmsg)
								multikillmsg = "WICKED SICK";
							else if (randmsg == 1)
								multikillmsg = "MONSTER KILL";
							else
								multikillmsg = "BOOM HEADSHOT";
						}
						else
							break;
						strncat(out, multikillmsg, sizeof(out));
						break;
					}
					case 'P': {
						char *multikillmsg;
						if (eth.killSpreeCount == 1) {
							multikillmsg = " ^2<< ^1Owned";
						}
						if (eth.killSpreeCount == 2) {
							multikillmsg = " ^2<< ^1Doublekill";
						}
						else if (eth.killSpreeCount == 3) {
							multikillmsg = " ^2<< ^1Triplekill";
						}
						else if (eth.killSpreeCount == 4) {
							multikillmsg = " ^2<< ^1Ultrakill";
						}
						else if (eth.killSpreeCount > 4) {
							multikillmsg = " ^2<< ^1Monsterkill";
						}
						else
							break;
						strncat(out, multikillmsg, sizeof(out));
						break;				
							}
						default:
							break;
					}
					a += 2;
				}
				else
					out[b] = in[a];
				b = strlen(out);
			}
		
			strncpy(in,out,sizeof(out));
			return in;
}


// Customizable spam
void killSpam() {
	char msg[256];
	char send[256];
	strncpy(msg,ksFormat,sizeof(msg));
	snprintf(send, sizeof(send), "say \"%s\"\n", Format(msg));
	syscall_CG_SendConsoleCommand(send);
}


// Auto demo record
void autoRecord() { 
	#define TMP_FILE_NAME "demo_in_progress"
	static qboolean demoState = qfalse;
	static int stateTime ;
	int ethRate;
	
	// Speed return if we don't want record demo
	if (!demoState && !seth.value[VAR_RECDEMO])
		return;

	if ((eth.cg_snap->ps.eFlags != EF_DEAD)
			&& (eth.cg_snap->ps.clientNum == eth.cg_clientNum)
			&& (eth.clientInfo[eth.cg_clientNum].team != TEAM_SPECTATOR)
			&& (eth.cg_snap->ps.stats[STAT_HEALTH] > 0)
			&& (eth.cg_snap->ps.pm_flags != PMF_FOLLOW)
			&& seth.value[VAR_RECDEMO]
			&& !demoState) {
		
		stateTime=0 ;
		// FIXME: check if a tmp demo file already exist
		syscall_CG_SendConsoleCommand("record " TMP_FILE_NAME "\n");
		stateTime = eth.cg_time;
		
		#ifdef ETH_DEBUG
			ethDebug("demo: start");
		#endif

		demoState = qtrue;
		
	} else if (((eth.cg_snap->ps.eFlags == EF_DEAD)
			|| (eth.cg_snap->ps.clientNum != eth.cg_clientNum)
			|| (eth.clientInfo[eth.cg_clientNum].team == TEAM_SPECTATOR)
			|| (eth.cg_snap->ps.stats[STAT_HEALTH] <= 0)
			|| (eth.cg_snap->ps.pm_flags == PMF_FOLLOW)
			|| (!seth.value[VAR_RECDEMO])) 
				&& demoState) {
				
		char buffFrom[MAX_OSPATH];
		char buffTo[MAX_OSPATH];

		// Get fsgame
		char fsGame[MAX_QPATH];
		syscall_CG_Cvar_VariableStringBuffer("fs_game", fsGame, sizeof(fsGame));

		// Get current time
		time_t ethTime = time(NULL);
		struct tm *localTime = localtime(&ethTime);

		orig_Cbuf_ExecuteText(EXEC_NOW, "stoprecord\n");

		stateTime = ((eth.cg_time - stateTime)/1000) ; 
		ethRate=(int)((((float)eth.autoDemoKillCount/(float)stateTime)*100.0f)/(float)(0.2)) ;
		
		#ifdef ETH_DEBUG
			ethDebug("demo: final stateTime:[%i]", stateTime) ;
			ethDebug("demo: kill acc:[%i]",ethRate);
			ethDebug("demo: stop");
		#endif
		
		// buffForm = /homepath/fsgame/
		syscall_CG_Cvar_VariableStringBuffer("fs_homepath", buffFrom, sizeof(buffFrom));
		sprintf(buffFrom, "%s/%s/demos/", buffFrom, fsGame);

		strncpy(buffTo, buffFrom, sizeof(buffFrom));

		// buffForm = /homepath/fsgame/demos/demoinprogress
		strcat(buffFrom, TMP_FILE_NAME ".dm_");

		// Contruct final file name, buffTo = /homepath/fsgame/demo/aaaa_mm_dd_hhmmss_mod_map_killcount
		sprintf(buffTo,"%s%i-%.2i-%.2i-%.2i%.2i%.2i_%s_%s_%ikills_%isec_%irate.dm_", buffTo, localTime->tm_year + 1900, localTime->tm_mon + 1,
				localTime->tm_mday, localTime->tm_hour, localTime->tm_min, localTime->tm_sec, fsGame,
				eth_Info_ValueForKey(eth.cgs_gameState->stringData + eth.cgs_gameState->stringOffsets[CS_SERVERINFO], "mapname"),
				eth.autoDemoKillCount, stateTime, ethRate);
		
		// Version of the demo
		if (!strcmp(sethET->version, "2.55")){
			strcat(buffFrom, "82");
			strcat(buffTo, "82");
		} else if(!strcmp(sethET->version, "2.56")){
			strcat(buffFrom, "83");
			strcat(buffTo, "83");
		} else {
			strcat(buffFrom, "84");
			strcat(buffTo, "84");
		}

		#ifdef ETH_DEBUG
			ethDebug("demo: got %i kills, need %i kills", eth.autoDemoKillCount, (int)seth.value[VAR_RECDEMO]);
		#endif

		// If we more than kills threshold then rename the demo
		if (eth.autoDemoKillCount >= seth.value[VAR_RECDEMO]) {
			#ifdef ETH_DEBUG
				ethDebug("demo: move [%s] to [%s]", buffFrom, buffTo);
			#endif
			rename(buffFrom, buffTo);
		// Else delete it
		} else {
			#ifdef ETH_DEBUG
				ethDebug("demo: delete [%s]", buffFrom, buffTo);
			#endif
			unlink(buffFrom);
		}

		eth.autoDemoKillCount = 0;
		demoState = qfalse;
	}
}

/*
==============================
et guid
==============================
*/

// Init cl_guid by environnement var
void loadCL_GUID(void) {
	char *cl_guid = getenv("CLGUID");
	if (cl_guid) {
		syscall_CG_Cvar_Set("cl_guid", cl_guid);
		// Delete env var so cl_guid is set once, and not each cg_init
		unsetenv("CLGUID");
	}
}

