""" mlflowによるlogging """

import argparse
import json
import statistics
from mlflow import set_tracking_uri, log_metric, log_param, log_artifact

def parse_arguments():
    parser = argparse.ArgumentParser(description=' ')
    parser.add_argument('-i', '--input_file', required=True)
    parser.add_argument('-r', '--result_file', required=True)
    parser.add_argument('-d', '--mlflow_tracking_uri', default='mlruns')
    args = parser.parse_args()
    return args

def main(args):
    set_tracking_uri(args.mlflow_tracking_uri) 
    log_artifact(args.input_file)
    with open(args.result_file) as f:
        scores = []
        for line in f:
            data = json.loads(line)
            score = float(data["score"])
            scores.append(score)
            log_metric(data["seed"], score)
        log_metric("mean", statistics.mean(scores))
        log_metric("median", statistics.median(scores))

if __name__ == '__main__':
    args = parse_arguments()
    main(args)

