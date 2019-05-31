//
// Created by dpons on 5/19/19.
//

#include "bkgSubtraction.h"
#include "Tracker.h"

bkgSubtraction::bkgSubtraction(const string &name, std::map<int, int> &instructions, vector<void*> sharedMemory):Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry *>(sharedMemory[0]);
}

bkgSubtraction::bkgSubtraction(const string &name, std::map<int, int> &instructions, void *sharedMemory) : Instruction(
        name, instructions, sharedMemory) {
    planimetry = static_cast<Planimetry * >(sharedMemory);

}

void bkgSubtraction::start(int socket, int port) {
    Instruction::start(socket, port);

    /* Type of object to detect
        1 -> person
        3 -> car
        4 -> motorbike
    */
    vector<int> objectToDetect = {0};

    backgroundSubtraction(objectToDetect);
}


/**
 * Manage background subtraction operations
 */
void bkgSubtraction::backgroundSubtraction(vector<int> toTrack) {
    float threshold = 0.5;
    int bkgSocket, bkgPid;

    // Do YOLO stuff and get results
    DarknetCalculator yoloCalculator = DarknetCalculator(threshold);

    while(true) {
        // Send start message
        bkgPid = startInstruction(1);
        bkgSocket = instructions[bkgPid];

        // Wait for relevant image
        cv::Mat image;

        if (!getAnswerImg(bkgSocket, image)) {
            log->writeLog("Stopping background subtraction process");
            break;
        }

        // Analyze results and start tracking if something is found
        int num_boxes = 0;
        int count = 0;
        detection * result = yoloCalculator.detect(image, &num_boxes);
        log->writeLog(string("Found: "));

        if (num_boxes > 0) {
            // Found something
            // TESTING PURPOSE: Iterate over predicted classes and print information.
            std::vector<std::string> labels = yoloCalculator.getLabels();
            for (int8_t i = 0; i < num_boxes; i++) {
                for (int j = 0; j < labels.size(); j++) {
                    if (result[i].prob[j] >= threshold) {
                        // More information is in each detections[i] item.
                        count++;
                        log->writeLog(string(labels[j]).append(" ").append(std::to_string(result[i].prob[j] * 100)));
                    }
                }
            }
            if (count == 0) log->writeLog(string("Nothing"));

            // Iterate over every result
            for (int8_t i = 0; i < num_boxes; i++) {
                // And over every object trained
                for (int j = 0; j < labels.size(); j++) {
                    if (std::find(toTrack.begin(), toTrack.end(), j) != toTrack.end() && result[i].prob[j] >= threshold) {
                        // Send instruction to start tracking
                        log->writeLog(string("Found ").append(labels[j]).append("! Starting tracking..."));

                        vector<string> trackingArgs;
                        trackingArgs.insert(trackingArgs.begin(), labels[j]);
                        trackingArgs.insert(trackingArgs.end(), to_string(j));
                        trackingArgs.insert(trackingArgs.end(), to_string(xImgSize));
                        trackingArgs.insert(trackingArgs.end(), to_string(yImgSize));

                        auto trackingInstr = new Tracker(string(labels[j]).append("-Tracker"), instructions, planimetry);
                        trackingInstr->start(nodeSocket, nodePort, trackingArgs);

                        // TODO Track only first object. To be fixed
                        break;
                    }
                }
            }

            cpp_free_detections(result, num_boxes);
        }

        if (instructions[bkgPid] != -1) {
            close(instructions[bkgPid]);
            instructions[bkgPid] = -1;
        }
    }

    // Stopped video stream or error occurred. Disconnecting last socket
    if (instructions[bkgPid] != -1) {
        close(instructions[bkgPid]);
        instructions[bkgPid] = -1;
    }
}
