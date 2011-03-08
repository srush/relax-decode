class Experiment:
  def __init__(self, desc):
    self.desc = desc
    self.model_type = desc.get("model_type",'nbayes')
    self.prefix = desc["prefix"]
    self.unk_exp_prefix =  desc["prefix"] + "_" + str(desc['unk_thres']) 
    self.full_exp_prefix = self.unk_exp_prefix +  "_" + str(int(10*desc['penalty'])) + "_" + self.model_type
    self.train = desc["train"]
    self.unk_thres = desc['unk_thres']
    #self.num_sentences = desc["num_sentences"]
    #self.num_cons = desc.get("num_constraints", 0)
    self._penalty = desc["penalty"]
    self.original_test = desc["original_test"]
    self._gold_file = desc["gold_file"]
    self._tagger = desc["tagger"]

  def constraint_type(self):
    return self.model_type

  def original_test_file(self):
    return "$SCARAB_DATA/" + self.original_test

  def full_test_file(self):
    return "$SCARAB_DATA/" + self.prefix

  def full_test_file_desc(self):
    return "$SCARAB_DATA/" + self.prefix + "_desc"

  def gold_file(self):
    return "$SCARAB_DATA/" + self._gold_file

  def train_file(self):
    return "$SCARAB_DATA/" + self.train 

  def unknown_words(self):
    return"$SCARAB_DATA/" + self.unk_exp_prefix

  def unknown_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix


  def tagger_model(self):
    return "$SCARAB_ROOT/third-party/stanford-postagger-2009-12-24/models/" + self._tagger

  def constraint_old_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix

  def mrf_link_prefix(self):
    return "$SCARAB_DATA/" + self.full_exp_prefix + "_link"

  def mrf_link_prefix_desc(self):
    return "$SCARAB_DATA/" + self.full_exp_prefix + "_link_desc"

  def brown_clusters(self):
    return "$SCARAB_DATA/paths"


  def constraint_mrf_prefix(self):
    return "$SCARAB_TMP/%s_mrf"%(self.full_exp_prefix)

  def result_prefix(self):
    return "$SCARAB_RESULTS/%s"%(self.full_exp_prefix)

  def lattice_prefix(self):
    return "$SCARAB_TMP/%s_lattice"%(self.unk_exp_prefix)

  def temporary_tags(self):
    return "$SCARAB_TMP/%s_tag"%(self.unk_exp_prefix)


  def temporary_margs(self):
    return "$SCARAB_TMP/%s_marginals"%(self.unk_exp_prefix)
  
  def num_sent(self):
    return self.num_sentences

  def num_constraints(self):
    return self.num_cons

  def penalty(self):
    return self._penalty


small_experiment = Experiment({
  "original_test": "brown_simple_1",
  "prefix": "brown_small_extra",
  "gold_file": "brown_small_extra_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0
  })

small_experiment2 = Experiment({
  "original_test": "brown_simple_1",
  "prefix": "brown_base",
  "gold_file": "brown_small_extra_test",
  "tagger": "left3words-wsj-0-18.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_dependency",
  "penalty" : 5.0
  })



# test_expermient = Experiment({
#     "prefix": "brown_extra20", 
#     #"exp_prefix": "brown_simple_1_100", 
#     "model_type": "nbayes",
#     "num_sentences" : 382,
#     "num_constraints" : 157,
    
#     "unk_thres" : 200,
#     "train" : "wsj_gold_dependency",
#     "penalty" : 2.0
#     })
