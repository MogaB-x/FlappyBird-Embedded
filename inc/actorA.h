#ifndef INC_ACTORA_H_
#define INC_ACTORA_H_

enum BlinkySignals {
    DUMMY_SIG = Q_USER_SIG,
	EV_SIG,
	MAX_PUB_SIG,          /* the last published signal */
    TIMEOUT_SIG,
	TIMEOUT_SIG1,
	ROT_UP_SIG,
	ROT_DN_SIG,
	ROT_BTN_SIG,
    MAX_SIG               /* the last signal */
};

void actorA_ctor(void);
extern QActive * const AO_actorA; /* opaque pointer */

#endif /* INC_ACTORA_H_ */