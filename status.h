#define STATUS_H

//dit is het bestand voor alle beslis logica.

enum {
    DEK_OPEN,
    DEK_DICHT,
    SLAGBOMEN_DICHT,
    SLAGBOMEN_SLUITEN,
    SLAGBOMEN_OPEN,
    SLAGBOMEN_RESET,
    NIET_BEZIG,
    BEZIG_OPENEN,
    BEZIG_SLUITEN,
    BEZIG_SLAGBOMEN_SLUITEN,
    BEZIG_NOOD,
};

const int TIJD_KNIPPERLICHT = 500;
const int WACHTIJD_BOOT = 5000;
const int WACHTIJD_SLAGBOMEN = 5000;
const int KRACHT_MOTOR_OPENEN = 50;
const int KRACHT_MOTOR_SLUITEN = 30;
const int TIJD_OPENEN = 1000;

bool reset = true;

int status_dek = DEK_OPEN;
int status_slagbomen = SLAGBOMEN_RESET;
int status_bezig = NIET_BEZIG;

int timer_lampen = 0;
int timer_boot = 0;
int timer_slagboom = 0;
int timer_open = 0;

void brug()
{
    while(1){
        input();

        if(status_dek == DEK_DICHT){
            //CONTROLEPANEELOPENLEDUIT;
        } else {
            //CONTROLEPANEELOPENLEDAAN;
        }


        if(is_er_een_boot()){
            CONTROLEPANEELBOTENLEDAAN;
        } else {
            CONTROLEPANEELBOTENLEDUIT;
        }

        if(is_er_verkeer()){
            CONTROLEPANEELVOETGANGERSLEDAAN;
        } else {
            CONTROLEPANEELVOETGANGERSLEDUIT;
        }

        //geen dedicated noodstop knops
        if (KNOPNOODSTOPINGEDRUKT){
            h_bridge_set_percentage(0);
            GELELEDSAAN;
            continue;
        } else {
            GELELEDSUIT;
        }

        if (BRUGDICHTLIMITINGEDRUKT) status_dek = DEK_DICHT;
        else status_dek = DEK_OPEN;

        if(reset){
            reset = false;
            DoorvaartVerbodenLeds();
            open_slagbomen();
        }

        if(schakelaar_modus){
            //automatische stand
            CONTROLEPANEELAUTOMATISCHLEDAAN;
            switch(status_dek){
                case DEK_DICHT:
                    if (is_er_een_boot()) open_brug();
                case DEK_OPEN:
                    if (is_er_een_boot() && !status_bezig == BEZIG_NOOD) timer_boot = millis;
                    if (abs(millis - timer_boot) > WACHTIJD_BOOT){
                        sluit_brug();
                    }
            }
        } else {
            //handmatige stand
            CONTROLEPANEELAUTOMATISCHLEDUIT;
            if(status_bezig == NIET_BEZIG){
                if(schakelaar_open) open_brug();
                else sluit_brug();
            }
        }

        if(is_wind_veilig()) CONTROLEPANEELWEERSOMSTANDIGHEDENLEDUIT;
        else {
            CONTROLEPANEELWEERSOMSTANDIGHEDENLEDAAN;

            if(schakelaar_modus){
                if(status_bezig == BEZIG_OPENEN)sluit_brug();
                if(status_dek == DEK_OPEN){
                    status_bezig == BEZIG_NOOD;
                    DoorvaartVerbodenLeds();
                }
            }
        }

        switch(status_bezig){
            case BEZIG_SLUITEN:
                //brug is dicht gegaan
                if(status_dek == DEK_DICHT){
                    h_bridge_set_percentage(0);
                    open_slagbomen();
                    DoorvaartVerbodenLeds();
                    status_bezig = NIET_BEZIG;
                }
            case BEZIG_OPENEN:
                //brug is open gegaan
                if (abs(millis - timer_open) >= TIJD_OPENEN){
                    h_bridge_set_percentage(0);
                    DoorvaartToegestaanLeds();
                    status_bezig = NIET_BEZIG;

                    if(schakelaar_modus){
                        timer_boot = millis;
                    }
                }
            case BEZIG_SLAGBOMEN_SLUITEN:
                if (status_slagbomen == SLAGBOMEN_DICHT) open_brug_volledig();
        }

        //knipperlichten
        if(status_slagbomen == SLAGBOMEN_DICHT || SLAGBOMEN_SLUITEN){
            if(abs(millis - timer_lampen) > TIJD_KNIPPERLICHT && abs(millis - timer_lampen) < TIJD_KNIPPERLICHT * 2){
                SLAGBOOMLED1AAN;
                SLAGBOOMLED2UIT;
            }

            if(abs(millis - timer_lampen) > TIJD_KNIPPERLICHT * 2){
                SLAGBOOMLED2AAN;
                SLAGBOOMLED1UIT;
                timer_lampen = millis;
            }
        }

        //wachten totdat brug vrij is
        if(status_slagbomen == SLAGBOMEN_SLUITEN){
            if(is_er_verkeer()){
                timer_slagboom = millis;
            }

            printf("%d", millis);

            if(abs(millis - timer_slagboom) >= WACHTIJD_SLAGBOMEN){
                sluit_slagbomen_volledig();
            }
        }
    }
}

void open_brug(){
    if(status_dek == DEK_OPEN) return;
    if(!is_wind_veilig()) return;
    if(is_er_verkeer()) return;

    if(!status_slagbomen == SLAGBOMEN_DICHT){
        sluit_slagbomen();
        status_bezig = BEZIG_SLAGBOMEN_SLUITEN;
        //wacht met het aanzetten van de motor totdat de slagbomen dicht zijn
        return;
    }

    open_brug_volledig();
}

void sluit_brug(){
    if(status_dek == DEK_DICHT) return;
    if(is_er_een_boot()) return;

    DoorvaartVerbodenLeds();

    status_bezig = BEZIG_SLUITEN;

    h_bridge_set_percentage(-KRACHT_MOTOR_SLUITEN);

    timer_open = millis;
}

void open_brug_volledig(){
    //gebruik alleen als slagbomen dicht zijn
    if(!is_wind_veilig()) return;
    status_bezig = BEZIG_OPENEN;
    h_bridge_set_percentage(KRACHT_MOTOR_OPENEN);
}

void sluit_slagbomen(){
    if(status_slagbomen == SLAGBOMEN_DICHT) return;

    //zet slagbomen op 45 graden voor waarschuwing
    servo1_set_percentage(0);
    servo2_set_percentage(0);

    timer_lampen = millis;
    timer_slagboom = millis;

    status_slagbomen = SLAGBOMEN_SLUITEN;
}

void sluit_slagbomen_volledig(){
    //toegepast na waarschuwen van verkeer

    servo1_set_percentage(100);
    servo2_set_percentage(100);

    status_slagbomen = SLAGBOMEN_DICHT;
}

void open_slagbomen(){
    if(status_slagbomen == SLAGBOMEN_OPEN) return;

    servo1_set_percentage(-100);
    servo2_set_percentage(-100);

    _delay_ms(500);

    status_slagbomen = SLAGBOMEN_OPEN;
}

