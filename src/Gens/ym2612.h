#ifndef _YM2612_H_
#define _YM2612_H_

#ifdef __cplusplus
extern "C" {
#endif

// Change it if you need to do long update
#define	MAX_UPDATE_LENGHT   2000

// Gens always uses 16 bits sound (in 32 bits buffer) and do the convertion later if needed.
#define OUTPUT_BITS         16

// VC++ inline
#define INLINE              __inline

typedef struct slot__ {
	int *DT;	// param�tre detune
	int MUL;	// param�tre "multiple de fr�quence"
	int TL;		// Total Level = volume lorsque l'enveloppe est au plus haut
	int TLL;	// Total Level ajusted
	int SLL;	// Sustin Level (ajusted) = volume o� l'enveloppe termine sa premi�re phase de r�gression
	int KSR_S;	// Key Scale Rate Shift = facteur de prise en compte du KSL dans la variations de l'enveloppe
	int KSR;	// Key Scale Rate = cette valeur est calcul�e par rapport � la fr�quence actuelle, elle va influer
				// sur les diff�rents param�tres de l'enveloppe comme l'attaque, le decay ...  comme dans la r�alit� !
	int SEG;	// Type enveloppe SSG
	int *AR;	// Attack Rate (table pointeur) = Taux d'attaque (AR[KSR])
	int *DR;	// Decay Rate (table pointeur) = Taux pour la r�gression (DR[KSR])
	int *SR;	// Sustin Rate (table pointeur) = Taux pour le maintien (SR[KSR])
	int *RR;	// Release Rate (table pointeur) = Taux pour le rel�chement (RR[KSR])
	int Fcnt;	// Frequency Count = compteur-fr�quence pour d�terminer l'amplitude actuelle (SIN[Finc >> 16])
	int Finc;	// frequency step = pas d'incr�mentation du compteur-fr�quence
				// plus le pas est grand, plus la fr�quence est a�gu (ou haute)
	int Ecurp;	// Envelope current phase = cette variable permet de savoir dans quelle phase
				// de l'enveloppe on se trouve, par exemple phase d'attaque ou phase de maintenue ...
				// en fonction de la valeur de cette variable, on va appeler une fonction permettant
				// de mettre � jour l'enveloppe courante.
	int Ecnt;	// Envelope counter = le compteur-enveloppe permet de savoir o� l'on se trouve dans l'enveloppe
	int Einc;	// Envelope step courant
	int Ecmp;	// Envelope counter limite pour la prochaine phase
	int EincA;	// Envelope step for Attack = pas d'incr�mentation du compteur durant la phase d'attaque
				// cette valeur est �gal � AR[KSR]
	int EincD;	// Envelope step for Decay = pas d'incr�mentation du compteur durant la phase de regression
				// cette valeur est �gal � DR[KSR]
	int EincS;	// Envelope step for Sustain = pas d'incr�mentation du compteur durant la phase de maintenue
				// cette valeur est �gal � SR[KSR]
	int EincR;	// Envelope step for Release = pas d'incr�mentation du compteur durant la phase de rel�chement
				// cette valeur est �gal � RR[KSR]
	int *OUTp;	// pointeur of SLOT output = pointeur permettant de connecter la sortie de ce slot � l'entr�e
				// d'un autre ou carrement � la sortie de la voie
	int INd;	// input data of the slot = donn�es en entr�e du slot
	int ChgEnM;	// Change envelop mask.
	int AMS;	// AMS depth level of this SLOT = degr� de modulation de l'amplitude par le LFO
	int AMSon;	// AMS enable flag = drapeau d'activation de l'AMS
} slot_;

typedef struct channel__ {
	int S0_OUT[4];			// anciennes sorties slot 0 (pour le feed back)
	int Old_OUTd;			// ancienne sortie de la voie (son brut)
	int OUTd;				// sortie de la voie (son brut)
	int LEFT;				// LEFT enable flag
	int RIGHT;				// RIGHT enable flag
	int ALGO;				// Algorythm = d�termine les connections entre les op�rateurs
	int FB;					// shift count of self feed back = degr� de "Feed-Back" du SLOT 1 (il est son unique entr�e)
	int FMS;				// Fr�quency Modulation Sensitivity of channel = degr� de modulation de la fr�quence sur la voie par le LFO
	int AMS;				// Amplitude Modulation Sensitivity of channel = degr� de modulation de l'amplitude sur la voie par le LFO
	int FNUM[4];			// hauteur fr�quence de la voie (+ 3 pour le mode sp�cial)
	int FOCT[4];			// octave de la voie (+ 3 pour le mode sp�cial)
	int KC[4];				// Key Code = valeur fonction de la fr�quence (voir KSR pour les slots, KSR = KC >> KSR_S)
	struct slot__ SLOT[4];	// four slot.operators = les 4 slots de la voie
	int FFlag;				// Frequency step recalculation flag
} channel_;

typedef struct ym2612__ {
	int Clock;			// Horloge YM2612
	int Rate;			// Sample Rate (11025/22050/44100)
	int TimerBase;		// TimerBase calculation
	int Status;			// YM2612 Status (timer overflow)
	int OPNAadr;		// addresse pour l'�criture dans l'OPN A (propre � l'�mulateur)
	int OPNBadr;		// addresse pour l'�criture dans l'OPN B (propre � l'�mulateur)
	int LFOcnt;			// LFO counter = compteur-fr�quence pour le LFO
	int LFOinc;			// LFO step counter = pas d'incr�mentation du compteur-fr�quence du LFO
						// plus le pas est grand, plus la fr�quence est grande
	int TimerA;			// timerA limit = valeur jusqu'� laquelle le timer A doit compter
	int TimerAL;
	int TimerAcnt;		// timerA counter = valeur courante du Timer A
	int TimerB;			// timerB limit = valeur jusqu'� laquelle le timer B doit compter
	int TimerBL;
	int TimerBcnt;		// timerB counter = valeur courante du Timer B
	int Mode;			// Mode actuel des voie 3 et 6 (normal / sp�cial)
	int DAC;			// DAC enabled flag
	int DACdata;		// DAC data
	double Frequence;	// Fr�quence de base, se calcul par rapport � l'horlage et au sample rate
	unsigned int Inter_Cnt;			// Interpolation Counter
	unsigned int Inter_Step;		// Interpolation Step
	struct channel__ CHANNEL[6];	// Les 6 voies du YM2612
	int REG[2][0x100];	// Sauvegardes des valeurs de tout les registres, c'est facultatif
						// cela nous rend le d�buggage plus facile
						// Kaneda:la valeur du DAC est fausse

} ym2612_;

/* Gens */
#ifdef GENS_KMOD
extern struct ym2612__ YM2612;
#endif

extern int YM2612_Enable;
extern int YM2612_Improv;
extern int DAC_Enable;
extern int *YM_Buf[2];
extern int YM_Len;

/* end */

int YM2612_Init(int clock, int rate, int interpolation);
int YM2612_End(void);
int YM2612_Reset(void);
int YM2612_Read(void);
int YM2612_Write(unsigned char adr, unsigned char data);
void YM2612_Update(int **buf, int length);
int YM2612_Save(unsigned char SAVE[0x200]);
int YM2612_Restore(unsigned char SAVE[0x200]);

/* Gens */

void YM2612_DacAndTimers_Update(int **buffer, int length);
void YM2612_Special_Update(void);

/* end */

// used for foward...
void Update_Chan_Algo0(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo1(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo2(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo3(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo4(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo5(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo6(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo7(channel_ *CH, int **buf, int lenght);

void Update_Chan_Algo0_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo1_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo2_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo3_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo4_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo5_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo6_LFO(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo7_LFO(channel_ *CH, int **buf, int lenght);

void Update_Chan_Algo0_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo1_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo2_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo3_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo4_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo5_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo6_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo7_Int(channel_ *CH, int **buf, int lenght);

void Update_Chan_Algo0_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo1_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo2_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo3_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo4_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo5_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo6_LFO_Int(channel_ *CH, int **buf, int lenght);
void Update_Chan_Algo7_LFO_Int(channel_ *CH, int **buf, int lenght);

// used for foward...
void Env_Attack_Next(slot_ *SL);
void Env_Decay_Next(slot_ *SL);
void Env_Substain_Next(slot_ *SL);
void Env_Release_Next(slot_ *SL);
void Env_NULL_Next(slot_ *SL);

#ifdef __cplusplus
};
#endif

#endif
