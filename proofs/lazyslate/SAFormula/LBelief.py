from fractions import Fraction

from .Belief import Belief

# fstring        -- String                -- String representation of formula (in s-expression style) (e.g. (Believes!4 alice now phi))
# fstring_no_ann -- String                -- String form with no likelihood
# justification  -- Justification || None -- Links the formula to its justification
# agent          -- String
# time           -- String
# formula        -- Formula               -- Sub-formula (object of modal operator)
class LBelief(Belief):

  def __init__(self, fstring, justification, agent, time, formula, likelihood, probability):
    super().__init__(fstring, justification, agent, time, formula)
    self.likelihood = likelihood
    self.probability = probability

    if isinstance(formula, LBelief): subf = formula.fstring_no_ann
    else:                            subf = str(formula)

    self.fstring_no_ann = "(Believes! " + agent + " " + time + " " + subf + ")"



  @classmethod
  def from_string(cls, fstring, justification=None):
    from .Parser import parse_fstring # Needs to be in function to avoid circular import

    # Split off the agent and time, leaving the sub-formula intact
    args = fstring.split(maxsplit=3)

    agent = args[1]
    time  = args[2]
    formula = parse_fstring(args[3][:-1]) # Pass sub-formula to parser

    # Defaults in case either are not set later
    likelihood = ''
    probability = ''

    # The part of the string containing the likelihood and/or probability
    uncertainty = fstring.split()[0].split('!')[1]

    # Default: Belief has likelihood, no probability
    # e.g. (Believes!4 a t phi)
    try:
      likelihood = int(uncertainty) 

    # If the exception hits, we may be in the other case, or may have an improperly formatted belief
    # e.g. (Believes![l=4,p=1/3] a t phi)
    except ValueError:

      try:
        if uncertainty[0] == '[' and uncertainty[-1] == ']':
          uncertainty = uncertainty[1:-1] # Remove square brackets
          for x in uncertainty.split(','):
            y = x.split('=')[1]
            if   x.startswith('l'): likelihood  = int(y)
            elif x.startswith('p'): probability = Fraction(y)

        else:
          print("Improperly formatted LBelief 1")
          exit(1)

      except ValueError:
        print("Improperly formatted LBelief 2")
        exit(1)
      

    return cls(fstring, justification, agent, time, formula, likelihood, probability)



  @classmethod
  def from_args(cls, agent, time, formula, likelihood='', justification=None, probability=''):
    if probability == '':  fstring = "(Believes!" + str(likelihood) + " " + agent + " " + time + " " + str(formula) + ")"
    elif likelihood == '': fstring = "(Believes![p=" + str(probability) + "] " + agent + " " + time + " " + str(formula) + ")"
    else:                  fstring = "(Believes![l=" + str(likelihood) + ",p=" + str(probability) + "] " + agent + " " + time + " " + str(formula) + ")"

    return cls(fstring, justification, agent, time, formula, likelihood, probability)



  def is_annotated(self):
    return True



  def __str__(self):
    return super().__str__()



  def __eq__(self, other):
    return super().__eq__(other) and self.likelihood == other.likelihood and self.probability == other.probability



  def __hash__(self):
    return super().__hash__()

