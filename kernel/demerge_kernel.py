import pandas as pd

demerge_csv = pd.read_csv("/home/zia/studies/algeng_wce_solver/build/out.csv")

demerge_csv.loc[
    ((demerge_csv['status_ui'] == "M") | (demerge_csv['status_vi'] == "M")) &               # if anyone of ui or vi is modified
    (demerge_csv["status_mi"] == 'U'),                                                      # and mi is not modified
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    ((demerge_csv['status_ui'] == "M") & (demerge_csv['status_vi'] == "M")) &               # if both ui and vi are modified
    (demerge_csv['cost_ui'].str[0] != demerge_csv['cost_vi'].str[0]),                       # and both have different connection type
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    ((demerge_csv['status_ui'] == "M") & (demerge_csv['status_vi'] == "M")) &               # if both ui and vi are modified
    (demerge_csv['cost_ui'].str[0] == demerge_csv['cost_vi'].str[0]) &                      # and both ui and vi have same connection type
    (demerge_csv['cost_mi'].str[0] != demerge_csv['cost_ui'].str[0]),                       # and mi type is different than ui or vi
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    (demerge_csv['status_ui'] != demerge_csv['status_vi']) &                                                            # if one of ui or vi is modified
    ( ((demerge_csv['status_ui'] == 'M') & (demerge_csv['cost_mi'].str[0] != demerge_csv['cost_ui'].str[0])) |          # if ui is modified and mi and ui have different connection types
    ((demerge_csv['status_vi'] == 'M') & (demerge_csv['cost_mi'].str[0] != demerge_csv['cost_vi'].str[0])) ),           # if vi is modified and mi and vi have different connection types
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    (demerge_csv['cost_mi'].str[1] == 'z') &                                                # if mi is zero
    ((demerge_csv['cost_ui'].str[1] != 'z') | (demerge_csv['cost_vi'].str[1] != 'z')),          # and ui or vi is not zero
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    ((demerge_csv['cost_ui'].str[1] == 'z') & (demerge_csv['cost_vi'].str[1] == 'z')) &          # and ui or vi is not zero
    (demerge_csv['cost_mi'].str[1] != 'z'),                                                # if mi is zero
    'new_cost_ui'] = 'NP'
demerge_csv.loc[
    ((demerge_csv['status_ui'] == "U") & (demerge_csv['status_vi'] == "U")) &                                           # if both ui and vi are modified
    (demerge_csv['cost_ui'].str[0] == demerge_csv['cost_vi'].str[0]) &                                                  # and both ui and vi have same connection type
    (((demerge_csv['cost_mi'].str[0] == demerge_csv['cost_ui'].str[0]) | (demerge_csv['status_mi'] != 'M')) &           # either mi should be modified and mi and ui have different connection type
     ((demerge_csv['cost_mi'].str[0] != demerge_csv['cost_ui'].str[0]) | (demerge_csv['status_mi'] != 'U'))),           # or mi is unmodified and mi and ui have same connection type
    'new_cost_ui'] = 'NP'

#print(demerge_csv.shape[0] - demerge_csv[demerge_csv.new_cost_ui == 'NP'].shape[0])
# after filtering

demerge_csv.loc[
    (demerge_csv.status_ui == 'M') & (demerge_csv.status_vi == 'M') & (demerge_csv.new_cost_ui != 'NP'), 'new_cost_ui'
] = 'RS'

demerge_csv.loc[
    (demerge_csv.status_ui == 'U') & (demerge_csv.status_vi == 'U') &                   # if both ui and vi are unmodified
    (demerge_csv.cost_ui.str[0] == demerge_csv.cost_vi.str[0]) &                        # if ui and vi have same connectivity type
    (demerge_csv.cost_mi.str[0] == demerge_csv.cost_ui.str[0]) &                        # if mi and ui/vi have same connectivity type
    (demerge_csv.new_cost_ui != 'NP'), 'new_cost_ui'
] = 'RS'

demerge_csv.loc[
    (demerge_csv.status_ui != demerge_csv.status_vi) &
    (demerge_csv.cost_ui.str[0] != demerge_csv.cost_vi.str[0]) &
    (demerge_csv.cost_mi.str[0] == demerge_csv.cost_ui.str[0]) &
    (demerge_csv.new_cost_ui != 'NP'),
    'new_cost_vi'
] = 'flip_vi'

demerge_csv.loc[
    (demerge_csv.status_ui != demerge_csv.status_vi) &
    (demerge_csv.cost_ui.str[0] != demerge_csv.cost_vi.str[0]) &
    (demerge_csv.cost_mi.str[0] == demerge_csv.cost_vi.str[0]) &
    (demerge_csv.new_cost_ui != 'NP'),
    'new_cost_ui'
] = 'flip_ui'

demerge_csv.loc[
    (demerge_csv.status_ui != demerge_csv.status_vi) &
    (demerge_csv.cost_ui.str[0] == demerge_csv.cost_vi.str[0]) &
    (demerge_csv.cost_mi.str[0] == demerge_csv.cost_ui.str[0]) &
    (demerge_csv.new_cost_ui != 'NP'),
    'new_cost_ui'
] = 'RS'

demerge_csv.loc[
    (demerge_csv.status_ui == 'U') & (demerge_csv.status_vi == 'U') &                   # if both ui and vi are unmodified
    (demerge_csv.cost_ui.str[0] != demerge_csv.cost_mi.str[0]) &                        # if ui and vi have same connectivity type
    (demerge_csv.new_cost_ui != 'NP'), 'new_cost_ui'
] = 'flipui'

demerge_csv.loc[
    (demerge_csv.status_ui == 'U') & (demerge_csv.status_vi == 'U') &                   # if both ui and vi are unmodified
    (demerge_csv.cost_vi.str[0] != demerge_csv.cost_mi.str[0]) &                        # if ui and vi have same connectivity type
    (demerge_csv.new_cost_ui != 'NP'), 'new_cost_vi'
] = 'flipvi'

# processed_rows = demerge_csv[demerge_csv.new_cost_ui != 'NP']
# processed_rows.to_csv("/home/zia/studies/algeng_wce_solver/build/out_modified.csv", index=False)

demerge_csv.to_csv("/home/zia/studies/algeng_wce_solver/build/out_modified.csv", index=False)