class NeuralNetwork {
public:
	void Init() {
	}

    void ISR() {
    }

private:
};

NeuralNetwork NEURALNET_instance;

// App stubs
void NEURALNET_init() {
}

size_t NEURALNET_storageSize() {
    return 0;
}

size_t NEURALNET_save(void *storage) {
    return 0;
}

size_t NEURALNET_restore(const void *storage) {
    return 0;
}

void NEURALNET_isr() {
	return NEURALNET_instance.ISR();
}

void NEURALNET_handleAppEvent(OC::AppEvent event) {

}

void NEURALNET_loop() {
}

void NEURALNET_menu() {
    menu::DefaultTitleBar::Draw();
    graphics.setPrintPos(1,1);
    graphics.print("Neural Network") ;

    graphics.setPrintPos(1,15);
    graphics.print("Coming Late 2018!");

    graphics.setPrintPos(1,25);
    graphics.print("Meanwhile,");

    graphics.setPrintPos(1,35);
    graphics.print("Try TL Neuron");

    graphics.setPrintPos(1,45);
    graphics.print("in Hemisphere");
}

void NEURALNET_screensaver() {
    NEURALNET_menu();
}

void NEURALNET_handleButtonEvent(const UI::Event &event) {

}

void NEURALNET_handleEncoderEvent(const UI::Event &event) {
}
