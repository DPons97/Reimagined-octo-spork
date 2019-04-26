//
// Created by dpons on 4/25/19.
//

#include <darknet.h>

network * cpp_load_network(char *cfg, char *weights, int clear) {
    return load_network(cfg, weights, clear);
}

void cpp_set_batch_network(network *net, int b) {
    set_batch_network(net, b);
}

image cpp_make_image(int w, int h, int c) {
    return make_image(w, h, c);
}

image cpp_letterbox_image(image im, int w, int h) {
    return letterbox_image(im, w, h);
}

float *cpp_network_predict(network *net, float *input) {
    return network_predict(net, input);
}

detection *cpp_get_network_boxes(network *net, int w, int h, float thresh, float hier, int *map, int relative, int *num) {
    return get_network_boxes(net, w, h, thresh, hier, map, relative, num);
}

char **cpp_get_labels(char *filename) {
    return get_labels(filename);
}

void cpp_free_image(image m) {
    free_image(m);
}

void cpp_free_detections(detection *dets, int n) {
    free_detections(dets, n);
}