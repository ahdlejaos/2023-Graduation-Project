SELECT [bf_user].[ID], [bf_user].[NICKNAME]
FROM [Users] [bf_user], [UserStaticInfos] [bf_info]
WHERE [bf_info].[EMAIL] = 'iconer'
