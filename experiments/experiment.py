class Experiment:
  def __init__(self, desc):
    self.desc = desc
    self.model_type = desc.get("model_type",'nbayes')
    self.prefix = desc["prefix"]
    self.unk_exp_prefix =  desc["prefix"] + "_" + str(desc['unk_thres']) 
    self.full_exp_prefix = self.unk_exp_prefix +  "_" + str(int(desc['penalty'])) + "_" + self.model_type
    self.train = desc["train"]
    self.unk_thres = desc['unk_thres']
    self.num_sentences = desc["num_sentences"]
    self.num_cons = desc.get("num_constraints", 0)
    self._penalty = desc["penalty"]

  def constraint_type(self):
    return self.model_type

  def original_simple_file(self):
    return "$SCARAB_DATA/" + self.prefix

  def gold_file(self):
    return "$SCARAB_DATA/" + self.prefix + "_gold"

  def train_file(self):
    return "$SCARAB_DATA/" + self.train 

  def unknown_words(self):
    return"$SCARAB_DATA/" + self.unk_exp_prefix

  def unknown_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix


  def constraint_old_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix

  def mrf_link_prefix(self):
    return "$SCARAB_DATA/" + self.full_exp_prefix + "_link"

  def constraint_mrf_prefix(self):
    return "$SCARAB_TMP/%s_mrf"%(self.full_exp_prefix)

  def result_prefix(self):
    return "$SCARAB_RESULTS/%s"%(self.full_exp_prefix)

  def lattice_prefix(self):
    return "$SCARAB_TMP/%s_lattice"%(self.unk_exp_prefix)

  def temporary_tags(self):
    return "$SCARAB_TMP/%s_tag"%(self.unk_exp_prefix)
  
  def num_sent(self):
    return self.num_sentences

  def num_constraints(self):
    return self.num_cons

  def penalty(self):
    return self._penalty

# class Experiment:
#   def __init__(self):
#     pass

#   def 





small_brown_3 = Experiment({
  "prefix": "brown_simple_1", 
  #"exp_prefix": "brown_simple_1_3", 
  "num_sentences" : 62,
  "unk_thres" : 3,
  "train" : "wsj_gold_dependency" ,
  "penalty" : 10.0
})


small_brown_10 = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_10", 
    "num_sentences" : 62,
    "unk_thres" : 10,
    "train" : "wsj_gold_dependency",
    "penalty" : 1.0
    })

small_brown_10_hard = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_10", 
    "num_sentences" : 62,
    "unk_thres" : 10,
    "train" : "wsj_gold_dependency",
    "penalty" : 1000.0
    })

small_brown_50 = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_50", 
    "num_sentences" : 62,
    "unk_thres" : 50,
    "train" : "wsj_gold_dependency",
    "penalty" : 10.0 
    })


small_brown_100 = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_100", 
    "num_sentences" : 62,
    "unk_thres" : 100,
    "train" : "wsj_gold_dependency",
    "penalty" : 1.0
    })

small_brown_100_hard = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_100", 
    "num_sentences" : 62,
    "unk_thres" : 100,
    "train" : "wsj_gold_dependency",
    "penalty" : 1000.0
    })


small_brown_200 = Experiment({
    "prefix": "brown_simple_1", 
    #"exp_prefix": "brown_simple_1_100", 
    "model_type": "nbayes",
    "num_sentences" : 62,
    "num_constraints" : 140,
    "unk_thres" : 200,
    "train" : "wsj_gold_dependency",
    "penalty" : 2.0
    })

small_brown_extra_200 = Experiment({
    "prefix": "brown_extra", 
    #"exp_prefix": "brown_simple_1_100", 
    "model_type": "nbayes",
    "num_sentences" : 91,
    "num_constraints" : 157,
    "unk_thres" : 200,
    "train" : "wsj_gold_dependency",
    "penalty" : 2.0
    })


small_brown_200_potts = Experiment({
    "prefix": "brown_simple_1", 
    "model_type": "potts",
    #"exp_prefix": "brown_simple_1_100", 
    "num_sentences" : 62,
    "num_constraints" : 140,
    "unk_thres" : 200,
    "train" : "wsj_gold_dependency",
    "penalty" : 2.0
    })

small_brown_500_potts = Experiment({
    "prefix": "brown_simple_1", 
    "model_type": "potts",
    "num_sentences" : 62,
    "num_constraints" : 184,
    "unk_thres" : 500,
    "train" : "wsj_gold_dependency",
    "penalty" : 2.0
    })

small_brown_500 = Experiment({
    "prefix": "brown_simple_1", 
    "num_sentences" : 62,
    "num_constraints" : 184,
    "unk_thres" : 500,
    "train" : "wsj_gold_dependency",
    "penalty" : 2.0
    })


