// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
