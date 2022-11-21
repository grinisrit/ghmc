from docs.quant.utils.optlib.Grid import *
from docs.quant.utils.optlib.numerical_utils import *
# TODO: add comments


class Solver(Grid):

    def __init__(self,
                 underlying: Underlying,
                 option: Option,
                 xSteps: int,
                 tSteps: int,
                 xLeft=-3.0,
                 xRight=3.0):

        super().__init__(underlying, option, xSteps, tSteps, xLeft, xRight)

    def _setBounds(self):
        self._net = set_bounds(net=self.net,
                               q=self.q,
                               t_array=self.tHeat,
                               x_array=self.xHeat,
                               call=self.option.call)

    def solve_crank_nickolson(self):
        self._setBounds()
        self._net = crank_nickolson_scheme(self.net, self.lamda)

    def solve_brennan_schwarz(self):
        if self.option.call:
            print('This algorithm is not used with Call Options')
        else:
            self._setBounds()
            self._net = brennan_schwartz_scheme(net=self.net,
                                                time_vector=self.tHeat,
                                                x_vector=self.xHeat,
                                                lambda_=self.lamda,
                                                q=self.q)
