""" mlflowによるlogging """

import argparse
import json
import statistics
import os
from mlflow import set_tracking_uri, log_metric, log_param, log_artifact

def parse_arguments():
    parser = argparse.ArgumentParser(description=' ')
    parser.add_argument('-i', '--input_file', required=True)
    parser.add_argument('-od', '--output_dir', required=True)
    parser.add_argument('-mtu', '--mlflow_tracking_uri', default='mlruns')
    args = parser.parse_args()
    return args

def main(args):
    set_tracking_uri(args.mlflow_tracking_uri) 
    log_artifact(args.input_file)
    with open(args.output_dir + "/result.txt") as f:
        scores = []
        for line in f:
            data = json.loads(line)
            score = float(data["score"])
            scores.append(score)
            log_metric(data["seed"], score)
            image_file = args.output_dir + "/{}.png".format(data["seed"])
            if os.path.isfile(image_file):
                log_artifact(image_file)
        log_metric("mean", statistics.mean(scores))
        log_metric("median", statistics.median(scores))

if __name__ == '__main__':
    args = parse_arguments()
    main(args)

