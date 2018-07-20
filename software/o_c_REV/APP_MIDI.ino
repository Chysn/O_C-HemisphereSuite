class MIDIInterface {
public:
	void Init() {
	}

    void ISR() {
    }

private:
};

MIDIInterface midi_instance;

// App stubs
void MIDI_init() {
}

size_t MIDI_storageSize() {
    return 0;
}

size_t MIDI_save(void *storage) {
    return 0;
}

size_t MIDI_restore(const void *storage) {
    return 0;
}

void MIDI_isr() {
	return midi_instance.ISR();
}

void MIDI_handleAppEvent(OC::AppEvent event) {

}

void MIDI_loop() {
}

void MIDI_menu() {
    menu::DefaultTitleBar::Draw();
    graphics.setPrintPos(1,1);
    graphics.print("MIDI Interface") ;

    graphics.setPrintPos(1,15);
    graphics.print("Coming August 2018!");

    graphics.setPrintPos(1,25);
    graphics.print("Meanwhile,");

    graphics.setPrintPos(1,35);
    graphics.print("Try MIDI In");

    graphics.setPrintPos(1,45);
    graphics.print("in Hemisphere");
}

void MIDI_screensaver() {
    MIDI_menu();
}

void MIDI_handleButtonEvent(const UI::Event &event) {

}

void MIDI_handleEncoderEvent(const UI::Event &event) {
}
