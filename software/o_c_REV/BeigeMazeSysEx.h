/*
 * System Exclusive Handler
 *
 * Wraps and unwraps system exclusive messages for Beige Maze products (Hemisphere, etc.).
 *
 * The license is the same as the rest of this project (MIT License)
 */
#ifndef BEIGEMAZE_SYSEX_

class SysEx {
public:
    SysEx(char target_id_, uint8_t size_) {
        target_id = target_id_;
        size = size_;
    }

    void Wrap(uint8_t *packet, int data[], char target_id) {
        int ix = 0;
        packet[ix++] = 0xf0;      // Start of SysEx
        packet[ix++] = 0x7d;      // Non-Commercial Manufacturer
        packet[ix++] = 0x62;      // Beige Maze
        packet[ix++] = target_id; // Target product
        for (int i = 0; i < size; i++)
        {
            uint16_t v = (uint16_t)data[i];
            for (int n = 0; n < 4; n++)
            {
                packet[ix++] = (v >> (4 * n)) & 0x0f;
            }
        }
        packet[ix++] = 0xf7; // End of SysEx
    }

    void Unwrap(int *container, uint8_t data[]) {
        int ix = 4;
        for (int i = 0; i < size; i++)
        {
            uint16_t v = 0;
            for (int n = 0; n < 4; n++)
            {
                v += data[ix++] << (4 * n);
            }
            container[i] = v;
        }
    }

    int getWrappedSize() {
        return size * 4 + 5;
    }

    bool verify(uint8_t data[]) {
        return (data[1] == 0x7d && data[2] == 0x62 && data[3] == target_id);
    }

private:
    int size;
    char target_id;
};

#endif /* BEIGEMAZE_SYSEX_ */
