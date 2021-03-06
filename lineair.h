bool slagbomen_zijn_open = false;

void open_brug(){
    if(is_er_verkeer()){
        return;
    }

    if(!is_wind_veilig()){
        return;
    }

    if(KNOPNOODSTOPINGEDRUKT){
        return;
    }

    knippper_slagboom_lichten();

    slagbomen_dicht();
    CONTROLEPANEELSLAGBOOMDICHTLEDAAN;

    _delay_ms(1000);

    h_bridge_set_percentage(50);
    _delay_ms(1000);
    h_bridge_set_percentage(0);
    CONTROLEPANEELOPENLEDAAN;

    if(!KNOPNOODSTOPINGEDRUKT){
        DoorvaartToegestaanLeds();
    } else {
        GELELEDSAAN;
    }

    _delay_ms(1000);

    while(1){
        input();
        leds();

        if(schakelaar_modus){
            if(!is_er_een_boot()){
                _delay_ms(10000);
                if(!is_er_een_boot()){
                    brug_dicht();
                    break;
                }
            }
        } else {
            if(KNOPOPENINGEDRUKT && !is_er_een_boot()){
                brug_dicht();
                break;
            }
        }


    }
}

void brug_dicht(){
    DoorvaartVerbodenLeds();
    h_bridge_set_percentage(-30);
    while(1){
        if(BRUGDICHTLIMITINGEDRUKT){
            h_bridge_set_percentage(0);
            CONTROLEPANEELOPENLEDUIT;

            _delay_ms(1000);

            slagbomen_open();

            CONTROLEPANEELSLAGBOOMDICHTLEDUIT;
            break;
        }
    }
}

void slagbomen_dicht(){
    if(is_er_verkeer()){
        return;
    }

    if(KNOPNOODSTOPINGEDRUKT){
        return;
    }

    servo1_set_percentage(100);
    servo2_set_percentage(100);
    slagbomen_zijn_open = false;
    CONTROLEPANEELSLAGBOOMDICHTLEDAAN;

    SLAGBOOMLED1UIT;
    SLAGBOOMLED2UIT;
}

void slagbomen_open(){
    if(KNOPNOODSTOPINGEDRUKT){
        return;
    }

    servo1_set_percentage(-100);
    servo2_set_percentage(-100);
    slagbomen_zijn_open = true;
    CONTROLEPANEELSLAGBOOMDICHTLEDUIT;
}

void knippper_slagboom_lichten(){

    for(int i = 0; i < 2; i++){
        SLAGBOOMLED1AAN;
        SLAGBOOMLED2UIT;
        _delay_ms(250);
        SLAGBOOMLED1UIT;
        SLAGBOOMLED2AAN;
        _delay_ms(250);
    }

    SLAGBOOMLED1AAN;


}

void leds(){
    if(is_er_verkeer()){
        CONTROLEPANEELVOETGANGERSLEDAAN;
    } else {
        CONTROLEPANEELVOETGANGERSLEDUIT;
    }

    if(is_er_een_boot()){
        CONTROLEPANEELBOTENLEDAAN;
    } else {
        CONTROLEPANEELBOTENLEDUIT;
    }

    if(is_wind_veilig()){
        CONTROLEPANEELWEERSOMSTANDIGHEDENLEDUIT;
    } else {
        CONTROLEPANEELWEERSOMSTANDIGHEDENLEDAAN;
    }

    if(schakelaar_modus){
        CONTROLEPANEELAUTOMATISCHLEDAAN;
    } else {
        CONTROLEPANEELAUTOMATISCHLEDUIT;
    }

    if(slagbomen_zijn_open){
        CONTROLEPANEELSLAGBOOMDICHTLEDUIT;
    } else {
        CONTROLEPANEELSLAGBOOMDICHTLEDAAN;
    }

}

void knoppen(){
    if(KNOPNOODSTOPINGEDRUKT){
        DoorvaartVerbodenLeds();
        GELELEDSAAN;
    } else {
        GELELEDSUIT;
    }

    if(schakelaar_modus){
        if(is_er_een_boot()){
            open_brug();
        }
    } else {
        if(KNOPOPENINGEDRUKT){
            open_brug();
        }
    }


   if(KNOPSLAGBOMENINGEDRUKT){
        if(slagbomen_zijn_open){
            knippper_slagboom_lichten();
            slagbomen_dicht();
        } else {
            slagbomen_open();
        }
        _delay_ms(500);
    }
}

void lineair(){
    slagbomen_open();
    DoorvaartVerbodenLeds();
    while(1){
        input();
        leds();
        knoppen();
    }
}
