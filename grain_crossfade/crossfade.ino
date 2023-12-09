// extern int xf;
extern int nSamples;
// extern int in;
// extern int out;
extern int buffer_xf[];

int crossfade() {
    int o2, i2, pos, w, inv, s;
    o2 = out;
    i2 = in;

    float xf_weight;
    float sample_weight;

    if (o2 == i2) {
        Serial.println("cross fading 0");

        pos = nSamples + xf;
        return buffer_xf[pos];
    } else if ((o2 < i2) && (o2 > (i2 - XFADE))) { // basically trying to blend what you are currently singing into what is in the xfade portion of the buffer
        Serial.println("cross fading 1");
        w   = in - out;  // Weight of sample (1-n)
        inv = XFADE - w; // Weight of xfade
        pos = nSamples + ((inv + xf) % XFADE);

        xf_weight = float(inv) / float(XFADE);
        sample_weight = 1-xf_weight;

        return (int) (float(buffer_xf[out]) * sample_weight + float(buffer_xf[pos]) * xf_weight);
        
    } else if (o2 > (i2 + nSamples - XFADE)) { // basically trying to blend what you are currently singing into what is in the xfade portion of the buffer
        Serial.println("cross fading 2");
      
        w   = in + nSamples - out;
        inv = XFADE - w;
        pos = nSamples + ((inv + xf) % XFADE);

        xf_weight = float(inv) / float(XFADE);
        sample_weight = 1-xf_weight;

        return (int) (float(buffer_xf[out]) * sample_weight + float(buffer_xf[pos]) * xf_weight);

    } else {
        Serial.println(XFADE);
        

        return buffer_xf[out];
    }

    return 0;
}