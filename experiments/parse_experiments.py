
class ParseExperiment:
  def __init__(self, original_test, prefix, gold_file, mrf_spec, model, penalty, training="wsj_gold_50", language="english"):
    self.original_test = original_test
    self.prefix = prefix
    self.gold_file = gold_file
    self.full_exp_prefix = prefix
    self._mrf_spec = mrf_spec
    self._model = model
    self._penalty = penalty
    self._model_pre = model.split('.')[0]
    self._training = training
    self.language = language
    
  def test_file(self):
    return "$SCARAB_DATA/%s"%self.original_test

  def training_file(self):
    return "$SCARAB_DATA/%s"%self._training
  
  def parse_out(self):
    return "$SCARAB_TMP/%s_%s_parse"%(self.original_test, self._model_pre)

  def hypergraph_prefix(self):
    return "$SCARAB_TMP/%s_%s_hypergraph"%(self.original_test, self._model_pre)

  def constraint_mrf_prefix(self):
    return "$SCARAB_TMP/%s_parse_mrf"%(self.full_exp_prefix)
  
  def self_train_out(self):
    return "$SCARAB_TMP/%s_self_train_parse"%(self.full_exp_prefix)

  def self_train_model(self):
    return "$SCARAB_TMP/%s_self_train_model"%(self.full_exp_prefix)

  def mrf_spec(self):
    return "$SCARAB_DATA/%s"%(self._mrf_spec)

  def model(self):
    return "$SCARAB_ROOT/third-party/mstparser/%s"%(self._model)

  def result_prefix(self):
    return "$SCARAB_RESULTS/%s"%(self.full_exp_prefix)

  def penalty(self):
    return self._penalty


# small_parse_experiment = ParseExperiment(
#   original_test= "english_test.conll",
#   prefix= "parse_first",
#   gold_file= "english_test.conll",
#   mrf_spec= "parse_constraints",
#   model = "small.model")

parse_experiment = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_bigger",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_200",
  model = "small.model",
  penalty = 0.1)

parse_experiment2 = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_bigger2",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_200",
  model = "small.model",
  penalty = 0.5)


parse_experiment_big = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_star",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle2 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle2",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.5)

parse_experiment_wide = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_wide",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_size",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.1)


parse_experiment_gentle_size = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_size",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_size",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_small = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_gentle_mi_small",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)


parse_experiment_gentle_mi_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_bigger2 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_bigger2",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle_mihard_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mihard_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi_hard",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asym_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asym_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym2_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asym2_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.3)

parse_experiment_gentle_mi_asymroi_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asymroi_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym_roi",
  model = "small.model",
  penalty = 0.3)


parse_experiment_gentle_mi_asymlow_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asymlow_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym_punc",
  training = "wsj_gold_50",
  model = "small.model",
  penalty = 0.7)


def parse_dev_experiment_size(num, model_size, prefix ="", penalty=0.7):
  return ParseExperiment(
  original_test= "parse_dev_data/sec22_%s"%num,
  prefix= "parse_dev_%s_%s_%s"%(prefix, model_size, num),
  gold_file= "parse_dev_data/sec22_%s"%num,
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_%s.model"%model_size,
  training = "wsj_gold_%s"%model_size,
  penalty = penalty)

simple_parse = ParseExperiment(
  original_test= "parse_dev_data/sec_test",
  prefix= "parse_dev_small",
  gold_file= "parse_dev_data/sec_test",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_SO.model",
  training = "wsj_gold_50",
  penalty = 0.7)

def parse_language(setnum, language, size =50, penalty=0.5):
  return ParseExperiment(
  original_test= "parse_dev_%s/dev_%03d"%(language,setnum),
  prefix= "parse_dev_%s_%s_%s"%(language, size, setnum),
  gold_file= "parse_dev_%s/dev_%03d"%(language,setnum),
  mrf_spec= "parse_constraints_asym_punc",
  model = "%s_%s.model"%(language,size),
  training = "%s_%s"%(language,size),
  penalty = penalty,
  language = language)

genia_parse = ParseExperiment(
  original_test= "GENIA_5",
  prefix= "parse_genia",
  gold_file= "GENIA_5",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "GENIA_look",
  penalty = 0.2)

qtb_parse_01 = ParseExperiment(
  original_test= "sec22.conll",
  prefix= "parse_qtb_01",
  gold_file= "sec22.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.1)

qtb_parse_03 = ParseExperiment(
  original_test= "sec22.conll",
  prefix= "parse_qtb_03",
  gold_file= "sec22.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.3)

qtb_parse_04 = ParseExperiment(
  original_test= "sec22.conll",
  prefix= "parse_qtb_04",
  gold_file= "sec22.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.4)

qtb_parse_05 = ParseExperiment(
  original_test= "sec22.conll",
  prefix= "parse_qtb_05",
  gold_file= "sec22.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.5)

qtb_test_parse = ParseExperiment(
  original_test= "sec23.conll",
  prefix= "parse_qtb_test",
  gold_file= "sec23.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.5)

qtb_parse_07 = ParseExperiment(
  original_test= "sec22.conll",
  prefix= "parse_qtb_07",
  gold_file= "sec22.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_qtb_fixed.model",
  training = "question_bank_new_dependency_fixed",
  penalty = 0.7)

qtb_flip_parse = ParseExperiment(
  original_test= "question_bank_new_dependency_fixed",
  prefix= "parse_flip_qtb",
  gold_file= "question_bank_new_dependency_fixed",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.3)

qtb_flip_parse_5 = ParseExperiment(
  original_test= "question_bank_new_dependency_fixed",
  prefix= "parse_flip_qtb_05",
  gold_file= "question_bank_new_dependency_fixed",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.5)

qtb_flip_self_parse_5 = ParseExperiment(
  original_test= "question_bank_new_dependency_fixed",
  prefix= "parse_flip_qtb_self_05",
  gold_file= "question_bank_new_dependency_fixed",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "english_40000",
  penalty = 0.5)

qtb_flip_parse_7 = ParseExperiment(
  original_test= "question_bank_new_dependency_fixed",
  prefix= "parse_flip_qtb_07",
  gold_file= "question_bank_new_dependency_fixed",
  mrf_spec= "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.7)

fb_parse = ParseExperiment(
  original_test = "football_dev_fixed",
  prefix = "parse_fb",
  gold_file = "football_dev_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.5)


sb_parse = ParseExperiment(
  original_test = "switchboard_test_gold_dependency_fixed",
  prefix = "parse_sb",
  gold_file = "switchboard_test_gold_dependency_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.5)

sb_parse_self = ParseExperiment(
  original_test = "switchboard_test_gold_dependency_fixed",
  prefix = "parse_sb",
  gold_file = "switchboard_test_gold_dependency_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "english_40000",
  penalty = 0.5)

sb_parse_sb = ParseExperiment(
  original_test = "switchboard_test_gold_dependency_fixed",
  prefix = "parse_sb_sb",
  gold_file = "switchboard_test_gold_dependency_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_sb_fixed.model",
  training = "sb_five",
  penalty = 0.5)

sb_parse_flip_test = ParseExperiment(
  original_test = "sec23.conll",
  prefix = "parse_flip_sb_test",
  gold_file = "sec23.conll",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_sb_fixed.model",
  training = "sb_five",
  penalty = 0.5)

sb_parse_flip_test_self = ParseExperiment(
  original_test = "sec23.conll",
  prefix = "parse_flip_sb_test_self",
  gold_file = "sec23.conll",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_sb_fixed.model",
  training = "english_10000",
  penalty = 0.5)


sb_parse_7 = ParseExperiment(
  original_test = "switchboard_test_gold_dependency_fixed",
  prefix = "parse_sb_07",
  gold_file = "switchboard_test_gold_dependency_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.7)

sb_parse_3 = ParseExperiment(
  original_test = "switchboard_test_gold_dependency_fixed",
  prefix = "parse_sb_03",
  gold_file = "switchboard_test_gold_dependency_fixed",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.3)


sb_parse_tiny = ParseExperiment(
  original_test = "switchboard_tiny",
  prefix = "parse_sb_tiny",
  gold_file = "switchboard_tiny",
  mrf_spec = "parse_constraints_asym_punc",
  model = "english_full.model",
  training = "wsj_gold_five",
  penalty = 0.5)

german_tagging = ParseExperiment(
  original_test= "german_parse_dev_50_retagged",
  prefix= "parse_tagging_german",
  gold_file= "german_parse_dev_50_retagged",
  mrf_spec= "parse_constraints_asym_punc",
  model = "german_50.model",
  training = "german_50",
  penalty = 0.5,
  language = "german")

german_tagging_500 = ParseExperiment(
  original_test= "german_parse_dev_500_retagged",
  prefix= "parse_tagging_german_500",
  gold_file= "german_parse_dev_500_retagged",
  mrf_spec= "parse_constraints_asym_punc",
  model = "german_500.model",
  training = "german_500",
  penalty = 0.5,
  language = "german")

german_base_500 = ParseExperiment(
  original_test= "german_parse_dev",
  prefix= "parse_base_german_500",
  gold_file= "german_parse_dev",
  mrf_spec= "parse_constraints_asym_punc",
  model = "german_500.model",
  training = "german_500",
  penalty = 0.5,
  language = "german")


def parse_language_big(setnum, language, size =50, penalty=0.5):
  return ParseExperiment(
  original_test= "parse_big_dev_%s/dev_%03d"%(language,setnum),
  prefix= "parse_big_dev_%s_%s_%s"%(language, size, setnum),
  gold_file= "parse_big_dev_%s/dev_%03d"%(language,setnum),
  mrf_spec= "parse_constraints_asym_punc",
  model = "%s_%s.model"%(language,size),
  training = "%s_%s"%(language,size),
  penalty = penalty,
  language = language)

def test_parse_language(setnum, language, size =50, penalty=0.5, extended=False, split=False):
  test_dir = "parse_test_%s"%language
  pre = "parse_test"
  if extended:
    test_dir = "parse_test_%s_ext"%language
    pre = "parse_test_ext"
  test_file = "/test_"
  if split:
    pre += "_split"
    test_file += "split_"
  if language != "english":
    return ParseExperiment(
      original_test= test_dir+test_file+"%03d"%(setnum),
      prefix= pre + "_%s_%s_%03d"%(language, size, setnum),
      gold_file= test_dir + test_file+"_%03d"%(setnum),
      mrf_spec= "parse_constraints_asym_punc",
      model = "%s_%s.model"%(language,size),
      training = "%s_%s"%(language,size),
      penalty = penalty,
      language = language)
  else:
    return ParseExperiment(
      original_test= test_dir + test_file+"%03d"%(setnum),
      prefix= pre + "_%s_%s_%03d"%(language, size, setnum),
      gold_file= test_dir + test_file+"%03d"%(setnum),
      mrf_spec= "parse_constraints_asym_punc",
      model = "small_%s.model"%size,
      training = "wsj_gold_%s"%size,
      penalty = penalty,
      language = language)




def parse_dev_experiment(num):
  return ParseExperiment(
  original_test= "parse_dev_data/sec22_%s"%num,
  prefix= "parse_dev9_%s"%num,
  gold_file= "parse_dev_data/sec22_%s"%num,
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_SO.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_dev_full_experiment = ParseExperiment(
  original_test= "parse_dev_data/sec22",
  prefix= "parse_dev_full",
  gold_file= "parse_dev_data/sec22",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_SO.model",
  training = "wsj_gold_50",
  penalty = 0.7)

def parse_test_experiment_full(num, penalty = 0.3, modelsize=50):
  return ParseExperiment(
  original_test= "sec23_gold_ulab.conll",
  prefix= "parse_ext_test_%s_%s"%(modelsize),
  gold_file= "sec23_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_%s.model"%modelsize,
  training = "wsj_gold_%s"%modelsize,
  penalty = penalty)

def parse_test_experiment(num, penalty = 0.3, modelsize=50):
  return ParseExperiment(
  original_test= "parse_test_data/sec23_%s"%num,
  prefix= "parse_test3_%s_%s"%(num, modelsize),
  gold_file= "parse_test_data/sec23_%s"%num,
  mrf_spec= "parse_constraints_asym_punc",
  model = "small_%s.model"%modelsize,
  training = "wsj_gold_%s"%modelsize,
  penalty = penalty)


parse_dev = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger6",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)


parse_experiment_gentle_mi_asymlow_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger2 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger2",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger3 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger3",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger4 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger4",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger5 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger5",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger6 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger6",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger7 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger7",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)


parse_experiment_gentle_mi_asymlow_bigger8 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger8",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 1.0)

parse_experiment_strong_mi_asymlow_tiny = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_strong_mi_asymlow_tiny",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small_SO.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_strong_mi_asymlow_tiny2 = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_strong_mi_asymlow_tiny2",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_asym_punc",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)


parse_experiment_strong_mi_asymlow_bigger_100 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_strong_mi_asymlow_bigger_100",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small100.model",
  penalty = 0.7)

parse_experiment_strong_mi_asymlow_bigger_500 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_strong_mi_asymlow_bigger_500",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small500.model",
  penalty = 0.7)

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
