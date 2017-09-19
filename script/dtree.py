#coding=utf8
# ========================================================
#   Copyright (C) 2017 All rights reserved.
#   
#   filename : dtree.py
#   author   : ***
#   date     : 2017-09-19
#   desc     : 
# ======================================================== 

import os, sys

class DTree (object):

    def __init__(self, model_path, learning_rate):
        self.model_path    = model_path
        self.learning_rate = learning_rate
        self.model         = []
        self.__load_model()

    def __load_model(self):
        dtrees = os.listdir(self.model_path)
        for dtreef in dtrees:
            model = []
            model_file = "%s/%s" %(self.model_path, dtreef)
            f = open(model_file)
            f.readline()
            while True:
                line = f.readline()
                if not line:
                    break
                model.append(line.strip().split("\t"))
            f.close()
            self.model.append(model)

    def __get_score(self, instance, dtree):
        tree_len = len(dtree)
        current_index = 0
        while dtree[current_index][1] != "1":
            node_attr = dtree[current_index][2]
            node_val  = float(dtree[current_index][3])
            if instance.has_key(node_attr) and float(instance[node_attr]) >= node_val :
                current_index = int(dtree[current_index][6])
            else:
                current_index = int(dtree[current_index][7])
        return (float(dtree[current_index][4]), current_index)

    def predict(self, instance):
        '''
        instance : feature,value k-v dict
                   if binary model, feature value : 1.0
        return   : final predict score, leaf_node_index
        '''
        ret_score = 0.0
        ret_path  = []
        for dtree in self.model:
            (score, index) = self.__get_score(instance, dtree)
            ret_score += self.learning_rate * score
            ret_path.append((index, score))
        return (ret_score, ret_path)

# test by instance
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print >> sys.stderr, "%s instance modelpath"
    tree_model = DTree(sys.argv[2], 0.1)
    for line in file(sys.argv[1]):
        segs = line.strip().split("\t")
        segs = segs[1:]
        length = len(segs)
        names = [segs[x] for x in range(0, length, 2)]
        valus = [segs[x] for x in range(1, length, 2)]
        instance = {}
        for name, value in zip(names, valus):
            instance.setdefault(name, value)
        score, path = tree_model.predict(instance)
        print >> sys.stdout, "%.5f" %score
