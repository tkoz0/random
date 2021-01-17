module LambdaCalculus where
import Data.List

-- standard lambda calculus expression
-- expr -> var_name | \var_name.expr | (expr expr)
data Expr =
    ExprVar String
    | ExprLam String Expr
    | ExprApp Expr Expr
    deriving (Eq,Show)

-- lambda calculus with de brujin indexes
-- expr -> integer | \expr | (expr expr)
data DBIExpr =
    DBIExprVar Int
    | DBIExprLam DBIExpr
    | DBIExprApp DBIExpr DBIExpr
    deriving (Eq,Show)

-- creates a list of free variables in a normal lambda calculus expression
freeVars :: Expr -> [String]
freeVars e@(ExprVar v) = [v]
freeVars e@(ExprLam x e1) = filter (\y -> y /= x) (freeVars e1)
freeVars e@(ExprApp e1 e2) = nub ((freeVars e1) ++ (freeVars e2))

expr2str :: Expr -> String
expr2str e@(ExprVar v) = v
expr2str e@(ExprLam x e1) = "\\" ++ x ++ "." ++ (expr2str e1)
expr2str e@(ExprApp a b) = "(" ++ (expr2str a) ++ " " ++ (expr2str b) ++ ")"

dbiexpr2str :: DBIExpr -> String
dbiexpr2str e@(DBIExprVar i) = show i
dbiexpr2str e@(DBIExprLam e1) = "\\" ++ (dbiexpr2str e1)
dbiexpr2str e@(DBIExprApp a b) =
    "(" ++ (dbiexpr2str a) ++ " " ++ (dbiexpr2str b) ++ ")"

-- make functions for single steps of applicative/normal order
-- make functions for several steps
-- make functions to evaluate until nonreducible (might never terminate)
-- use tail recursion where possible

{-

module Interpreter where
import Control.Exception
import Debug.Trace
import Data.List
import qualified Data.Set as Set

type Name = String
data Expr = Var Name | Lambda Name Expr | App Expr Expr deriving (Eq,Show)

-- returns a deduplicated list of free variables in e
freeVars :: Expr -> [Name]
freeVars e = case e of
    -- var is free in itself
    Var v -> [v]
    -- x is not free
    Lambda x e1 -> filter (\y -> y /= x) (freeVars e1)
    -- combine free vars of each expression
    App e1 e2 -> nub ((freeVars e1) ++ (freeVars e2))

-- replace free occurrences of x with m
subst :: (Name,Expr) -> Expr -> Expr
subst (x,m) e = case e of
    -- replace x with m
    Var y -> if x == y then m else Var y
    -- make the substitution in each side of an application
    App e1 e2 -> App (subst (x,m) e1) (subst (x,m) e2)
    -- as required, replace y with the next fresh var (z)
    Lambda y e1 -> let z = head (freshVars [e1,m,(Var x)]) in
                   -- if bound then do not change expr
                   if x == y then Lambda y e1
                   -- do the variable renaming as instructed
                   else Lambda z (subst (x,m) (subst (y,(Var z)) e1))

-- applicative order step (uses leftmost redex)
appNF_OneStep :: Expr -> Maybe Expr
appNF_OneStep e = case e of
    -- cannot reduce variable
    Var x -> Nothing
    -- try to reduce expr part of lambda
    Lambda x e1 -> let e2 = appNF_OneStep e1 in
        case e2 of
            Nothing -> Nothing
            Just e3 -> Just (Lambda x e3)
        --if e2 == Nothing then Nothing else Lambda x e2
    -- try to reduce component exprs before reducing application
    App e1 e2 -> let e1r = appNF_OneStep e1 in let e2r = appNF_OneStep e2 in
        case e1r of
            Nothing ->
                case e2r of
                    Nothing -> -- try function application
                        case e1 of
                            Lambda x e3 -> Just (subst (x,e2) e3)
                            Var x -> Nothing
                            App e3 e4 -> Nothing
                    Just e2rr -> Just (App e1 e2rr)
            Just e1rr -> Just (App e1rr e2)

-- perform (up to) n reductions
appNF_n :: Int -> Expr -> Expr
appNF_n n e =
    if n == 0 then e
    else let er = appNF_OneStep e in
        case er of
            Nothing -> e -- nonreducible so terminate
            Just err -> appNF_n (n-1) err -- continue reduction

-- underscore var names
uVars = map (\x -> (show x) ++ "_") [1..]

-- nonrepeated list of free vars in exprs
allFree :: [Expr] -> Set.Set Name
allFree exprs =
    foldl Set.union Set.empty (map (\e -> Set.fromList (freeVars e)) exprs)

-- generate var names that are not free in any of the exprs
freshVars :: [Expr] -> [Name]
freshVars expr_li = filter (\x -> not (Set.member x (allFree expr_li))) uVars

-}
